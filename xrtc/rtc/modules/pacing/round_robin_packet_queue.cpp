//
// Created by faker on 2023/7/1.
//

#include "xrtc/rtc/modules/pacing/round_robin_packet_queue.h"

namespace xrtc {
    namespace {

// 两个视频流之间累计发送字节数的最大差值
        const webrtc::DataSize kMaxLeadingSize = webrtc::DataSize::Bytes(1400);

    } // namespace

    RoundRobinPacketQueue::QueuedPacket::QueuedPacket(int priority,
                                                      webrtc::Timestamp enqueue_time,
                                                      uint64_t enqueue_order,
                                                      std::unique_ptr<RtpPacketToSend> packet) :
            priority_(priority),
            enqueue_time_(enqueue_time),
            enqueue_order_(enqueue_order),
            owned_packet_(packet.release())
    {
    }

    RoundRobinPacketQueue::QueuedPacket::~QueuedPacket() {
    }

    RoundRobinPacketQueue::Stream::Stream() :
            size(webrtc::DataSize::Zero()),
            ssrc(0)
    {
    }

    RoundRobinPacketQueue::Stream::~Stream() {
    }

    RoundRobinPacketQueue::RoundRobinPacketQueue(webrtc::Timestamp start_time) :
            max_size_(kMaxLeadingSize),
            last_time_updated_(start_time)
    {
    }

    RoundRobinPacketQueue::~RoundRobinPacketQueue() {
    }

    void RoundRobinPacketQueue::Push(int priority,
                                     webrtc::Timestamp enqueue_time,
                                     uint64_t enqueue_order,
                                     std::unique_ptr<RtpPacketToSend> packet)
    {
        Push(QueuedPacket(priority, enqueue_time, enqueue_order,
                          std::move(packet)));
    }

    std::unique_ptr<RtpPacketToSend> RoundRobinPacketQueue::Pop() {
        // 获取优先级最高的流
        Stream* stream = GetHighestPriorityStream();
        const QueuedPacket& queued_packet = stream->packet_queue.top();

        // 一旦数据包从stream的队列中出列，stream的优先级可能会发生变化，需要更新
        // 先从map中删除该流的优先级
        stream_priorities_.erase(stream->priority_it);

        queue_time_sum_ -= (last_time_updated_ - queued_packet.EnqueueTime());

        webrtc::DataSize packet_size = PacketSize(queued_packet);
        // 更新stream累计发送的字节数
        // 大视频流 2000
        // 小视频流 50
        // 确保两个流之间累计发送的字节数最大不会相差kMaxLeadingSize
        stream->size = std::max(stream->size + packet_size, max_size_ - kMaxLeadingSize);
        max_size_ = std::max(stream->size, max_size_);

        std::unique_ptr<RtpPacketToSend> rtp_packet(queued_packet.rtp_packet());
        stream->packet_queue.pop();
        size_packets_ -= 1;
        size_ -= packet_size;

        // 重新计算stream的优先级
        if (stream->packet_queue.empty()) {
            stream->priority_it = stream_priorities_.end();
        }
        else {
            int priority = stream->packet_queue.top().Priority();
            stream->priority_it = stream_priorities_.emplace(
                    StreamPrioKey(priority, stream->size), stream->ssrc);
        }

        return rtp_packet;
    }

    bool RoundRobinPacketQueue::Empty() const {
        return size_packets_ == 0;
    }

    void RoundRobinPacketQueue::UpdateQueueTime(webrtc::Timestamp now) {
        if (now == last_time_updated_) {
            return;
        }

        webrtc::TimeDelta delta = now - last_time_updated_;
        queue_time_sum_ += webrtc::TimeDelta::Micros(delta.us() * size_packets_);
        last_time_updated_ = now;
    }

    webrtc::TimeDelta RoundRobinPacketQueue::AverageQueueTime() const {
        if (Empty()) {
            return webrtc::TimeDelta::Zero();
        }

        return queue_time_sum_ / size_packets_;
    }

    void RoundRobinPacketQueue::Push(const QueuedPacket& packet) {
        // 查找packet是否存在对应的stream
        auto stream_iter = streams_.find(packet.Ssrc());
        if (stream_iter == streams_.end()) { // Stream第一个数据包
            stream_iter = streams_.emplace(packet.Ssrc(), Stream()).first;
            // 新的数据尚未列入优先级排序
            stream_iter->second.priority_it = stream_priorities_.end();
            stream_iter->second.ssrc = packet.Ssrc();
        }

        Stream* stream = &stream_iter->second;
        // 调整流的优先级
        if (stream->priority_it == stream_priorities_.end()) {
            // 在map当中还没有进行该流的优先级排序
            stream->priority_it = stream_priorities_.emplace(StreamPrioKey(packet.Priority(), stream->size),
                                                             packet.Ssrc());
        }
        else if (packet.Priority() < stream->priority_it->first.priority) {
            stream_priorities_.erase(stream->priority_it);
            // 重新插入，更新优先级
            stream->priority_it = stream_priorities_.emplace(StreamPrioKey(packet.Priority(), stream->size),
                                                             packet.Ssrc());
        }

        UpdateQueueTime(packet.EnqueueTime());
        size_packets_ += 1;
        size_ += PacketSize(packet);
        stream->packet_queue.emplace(packet);
    }

    RoundRobinPacketQueue::Stream*
    RoundRobinPacketQueue::GetHighestPriorityStream() {
        uint32_t ssrc = stream_priorities_.begin()->second;
        auto stream_it = streams_.find(ssrc);
        return &stream_it->second;
    }

    webrtc::DataSize RoundRobinPacketQueue::PacketSize(const QueuedPacket& queued_packet) {
        return webrtc::DataSize::Bytes(queued_packet.rtp_packet()->payload_size() +
                                       queued_packet.rtp_packet()->padding_size());
    }

} // namespace xrtc

