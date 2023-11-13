#include "xrtc/rtc/modules/congestion_controller/goog_cc/delay_based_bwe.h"

namespace xrtc {
    namespace {

        constexpr webrtc::TimeDelta kStreamTimeout = webrtc::TimeDelta::Seconds(2);
        constexpr webrtc::TimeDelta kSendTimeGroupLength = webrtc::TimeDelta::Millis(5);

    } // namespace

    DelayBasedBwe::DelayBasedBwe() {
    }

    DelayBasedBwe::~DelayBasedBwe() {
    }

    DelayBasedBwe::Result DelayBasedBwe::IncomingPacketFeedbackVector(
            const webrtc::TransportPacketsFeedback& msg)
    {
        // 数据包按照到达时间进行排序
        auto packet_feedback_vector = msg.SortedByReceiveTime();
        if (packet_feedback_vector.empty()) {
            return DelayBasedBwe::Result();
        }

        for (const auto& packet_feedback : packet_feedback_vector) {
            IncomingPacketFeedback(packet_feedback, msg.feedback_time);
        }

        return DelayBasedBwe::Result();
    }

    void DelayBasedBwe::IncomingPacketFeedback(const webrtc::PacketResult& packet_feedback,
                                               webrtc::Timestamp at_time)
    {
        // 如果是第一次收到packet，需要创建计算包组时间差的对象
        // 如果长时间没有收到packet超过一定的阈值，需要重新创建对象
        if (last_seen_timestamp_.IsInfinite() ||
            at_time - last_seen_timestamp_ > kStreamTimeout)
        {
            video_inter_arrival_delta_ = std::make_unique<InterArrivalDelta>(
                    kSendTimeGroupLength);
        }

        last_seen_timestamp_ = at_time;

        size_t packet_size = packet_feedback.sent_packet.size.bytes();

        webrtc::TimeDelta send_time_delta = webrtc::TimeDelta::Zero();
        webrtc::TimeDelta recv_time_delta = webrtc::TimeDelta::Zero();
        int packet_size_delta = 0;

        bool calculated_delta = video_inter_arrival_delta_->ComputeDeltas(
                packet_feedback.sent_packet.send_time,
                packet_feedback.receive_time,
                at_time,
                packet_size,
                &send_time_delta, &recv_time_delta, &packet_size_delta);
    }

} // namespace xrtc