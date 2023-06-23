//
// Created by faker on 2023/6/19.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"

#include <modules/rtp_rtcp/source/byte_io.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"

namespace xrtc {

    const size_t kNaluShortStartSequenceSize = 3;
    const size_t kFuAHeaderSize = 2;
    const size_t kNaluHeaderSize = 1;
    const size_t kLengthFieldSize = 2;

// NAL头部的bit mask
    enum NalDef : uint8_t {
        kFBit = 0x80,
        kNriMask = 0x60,
        kTypeMask = 0x1F,
    };

// FU-Header bit mask
    enum FuDef : uint8_t {
        kSBit = 0x80,
        kEBit = 0x40,
        kRBit = 0x20,
    };

    RtpPacketizerH264::RtpPacketizerH264(
            rtc::ArrayView<const uint8_t> payload,
            const RtpPacketizer::Config& config) :
            config_(config)
    {
        for (const auto& nalu : FindNaluIndices(payload.data(), payload.size())) {
            input_fragments_.push_back(payload.subview(nalu.payload_start_offset, nalu.payload_size));
        }

        if (!GeneratePackets()) {
            num_packets_left_ = 0;
            while (!packets_.empty()) {
                packets_.pop();
            }
        }
    }

    size_t RtpPacketizerH264::NumPackets() {
        return num_packets_left_;
    }

    bool RtpPacketizerH264::NextPacket(RtpPacketToSend* rtp_packet) {
        if (packets_.empty()) {
            return false;
        }

        PacketUnit* packet = &packets_.front();
        if (packet->first_fragment && packet->last_fragment) {
            // 单个NALU包
            size_t packet_size = packet->source_fragment.size();
            uint8_t* buffer = rtp_packet->AllocatePayload(packet_size);
            memcpy(buffer, packet->source_fragment.data(), packet_size);
            packets_.pop();
            input_fragments_.pop_front();
        }
        else if (packet->aggregated) {
            // STAP-A
            NextAggregatedPacket(rtp_packet);
        }
        else {
            // FU-A
            NextFragmentPacket(rtp_packet);
        }

        --num_packets_left_;
        rtp_packet->SetMarker(packets_.empty());

        return true;
    }

    std::vector<NaluIndex> RtpPacketizerH264::FindNaluIndices(const uint8_t* buffer,
                                                              size_t buffer_size)
    {
        // 起始码有3个或者4个字节
        std::vector<NaluIndex> sequences;
        if (buffer_size < kNaluShortStartSequenceSize) {
            return sequences;
        }

        size_t end = buffer_size - kNaluShortStartSequenceSize;
        for (size_t i = 0; i < end; ) {
            // 查找NALU的起始码
            if (buffer[i + 2] > 1) {
                i += 3;
            }
            else if (buffer[i + 2] == 1) {
                if (buffer[i] == 0 && buffer[i + 1] == 0) {
                    // 找到了一个起始码
                    NaluIndex index = { i, i + 3, 0 };
                    // 是否是4字节的起始码
                    if (index.start_offset > 0 && buffer[index.start_offset - 1] == 0) {
                        --index.start_offset;
                    }

                    auto it = sequences.rbegin();
                    if (it != sequences.rend()) {
                        it->payload_size = index.start_offset - it->payload_start_offset;
                    }

                    sequences.push_back(index);
                }

                i += 3;
            }
            else if (buffer[i + 2] == 0) {
                i += 1;
            }
        }

        // 计算最后一个NALU的payload size
        auto it = sequences.rbegin();
        if (it != sequences.rend()) {
            it->payload_size = buffer_size - it->payload_start_offset;
        }

        return sequences;
    }

    bool RtpPacketizerH264::GeneratePackets() {
        // 遍历从buffer当中提取的NALU
        for (size_t i = 0; i < input_fragments_.size(); ) {
            size_t fragment_len = input_fragments_[i].size();
            // 首先获取该NALU容纳负载的最大容量
            size_t single_packet_capacity = config_.limits.max_payload_len;
            if (input_fragments_.size() == 1) {
                single_packet_capacity -= config_.limits.single_packet_reduction_len;
            }
            else if (i == 0) { // 第一个包
                single_packet_capacity -= config_.limits.first_packet_reduction_len;
            }
            else if (i + 1 == input_fragments_.size()) { // 最后一个包
                single_packet_capacity -= config_.limits.last_packet_reduction_len;
            }

            if (fragment_len > single_packet_capacity) { // 分片打包
                if (!PacketizeFuA(i)) {
                    return false;
                }
                ++i;
            }
            else {
                i = PacketizeStapA(i);
            }
        }
        return true;
    }

// 分片打包
    bool RtpPacketizerH264::PacketizeFuA(size_t fragment_index) {
        rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
        PayloadLimits limits = config_.limits;
        // 预留FU-A头部的空间
        limits.max_payload_len -= kFuAHeaderSize;
        // 如果是多个NALU
        if (input_fragments_.size() != 1) {
            if (fragment_index == input_fragments_.size() - 1) {
                // 这里面只会包含中间的包和最后一个包
                limits.single_packet_reduction_len = limits.last_packet_reduction_len;
            }
            else if (fragment_index == 0) {
                // 这里面只包含第一个包和中间包
                limits.single_packet_reduction_len = limits.first_packet_reduction_len;
            }
            else {
                // 只包含中间包
                limits.single_packet_reduction_len = 0;
            }
        }

        if (fragment_index != 0) {
            // 第一个包不可能包含在这个NALU
            limits.first_packet_reduction_len = 0;
        }

        if (fragment_index != input_fragments_.size() - 1) {
            // 最后一个包不可能出现在这个NALU
            limits.last_packet_reduction_len = 0;
        }

        size_t payload_left = fragment.size() - kNaluHeaderSize;
        // 负载的起始偏移量
        size_t offset = kNaluHeaderSize;
        // 将负载大小分割成大体相同的几个部分
        std::vector<int> payload_sizes = SplitAboutEqual(payload_left, limits);
        if (payload_sizes.empty()) {
            return false;
        }

        for (size_t i = 0; i < payload_sizes.size(); ++i) {
            size_t packet_length = payload_sizes[i];
            packets_.push(PacketUnit(
                    fragment.subview(offset, packet_length),
                    i == 0,
                    i == payload_sizes.size() - 1,
                    false, fragment[0]
            ));

            offset += packet_length;
        }

        num_packets_left_ += payload_sizes.size();

        return true;
    }

    size_t RtpPacketizerH264::PacketizeStapA(size_t fragment_index) {
        size_t payload_size_left = config_.limits.max_payload_len;
        if (input_fragments_.size() == 1) {
            payload_size_left -= config_.limits.single_packet_reduction_len;
        }
        else if (fragment_index == 0) {
            // 第一个包
            payload_size_left -= config_.limits.first_packet_reduction_len;
        }

        int aggregated_fragment = 0;
        int fragment_header_length = 0;
        rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
        ++num_packets_left_;

        auto payload_size_needed = [&] {
            size_t fragment_size = fragment.size() + fragment_header_length;
            if (input_fragments_.size() == 1) {
                return fragment_size;
            }

            if (fragment_index == input_fragments_.size() - 1) {
                return fragment_size + config_.limits.last_packet_reduction_len;
            }

            return fragment_size;
        };

        while (payload_size_left >= payload_size_needed()) {
            packets_.push(PacketUnit(
                    fragment,
                    aggregated_fragment == 0,
                    false,
                    true,
                    fragment[0]));
            payload_size_left -= fragment.size();
            payload_size_left -= fragment_header_length;

            fragment_header_length = kLengthFieldSize;
            if (0 == aggregated_fragment) {
                fragment_header_length += (kNaluHeaderSize + kLengthFieldSize);
            }

            ++aggregated_fragment;

            // 继续聚合下一个包
            ++fragment_index;
            if (fragment_index == input_fragments_.size()) {
                break;
            }

            fragment = input_fragments_[fragment_index];
        }

        packets_.back().last_fragment = true;

        return fragment_index;
    }

    void RtpPacketizerH264::NextAggregatedPacket(RtpPacketToSend* rtp_packet) {
        size_t rtp_packet_capacity = rtp_packet->FreeCapacity();
        uint8_t* buffer = rtp_packet->AllocatePayload(rtp_packet_capacity);
        PacketUnit* packet = &packets_.front();
        // 写入STAP-A header
        buffer[0] = (packet->header & (kFBit | kNriMask)) | NaluType::kStapA;
        size_t index = kNaluHeaderSize;
        bool is_last_fragment = packet->last_fragment;
        // 写入NALU
        while (packet->aggregated) {
            rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
            // 写入NALU length field
            webrtc::ByteWriter<uint16_t>::WriteBigEndian(&buffer[index], (uint16_t)fragment.size());
            index += kLengthFieldSize;
            // 写入NALU
            memcpy(&buffer[index], fragment.data(), fragment.size());
            index += fragment.size();
            packets_.pop();
            input_fragments_.pop_front();
            if (is_last_fragment) {
                break;
            }

            packet = &packets_.front();
            is_last_fragment = packet->last_fragment;
        }

        // index指向哪里就是负载的大小
        rtp_packet->SetPayloadSize(index);
    }

    void RtpPacketizerH264::NextFragmentPacket(RtpPacketToSend* rtp_packet) {
        PacketUnit* packet = &packets_.front();
        // 构造FU-Indicator
        uint8_t fu_indicator = (packet->header & (kFBit | kNriMask)) |
                               NaluType::kFuA;
        // 构造FU-Header
        uint8_t fu_header = 0;
        fu_header |= (packet->first_fragment ? kSBit : 0);
        fu_header |= (packet->last_fragment ? kEBit : 0);
        // 提取原始的NALU type
        uint8_t type = packet->header & kTypeMask;
        fu_header |= type;
        // 写入到rtp buffer
        rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
        uint8_t* buffer = rtp_packet->AllocatePayload(kFuAHeaderSize + fragment.size());
        buffer[0] = fu_indicator;
        buffer[1] = fu_header;
        memcpy(buffer + kFuAHeaderSize, fragment.data(), fragment.size());
        packets_.pop();
        if (packet->last_fragment) {
            input_fragments_.pop_front();
        }
    }

} // namespace xrtc
