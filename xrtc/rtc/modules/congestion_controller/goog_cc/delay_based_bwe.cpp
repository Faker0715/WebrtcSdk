#include "xrtc/rtc/modules/congestion_controller/goog_cc/delay_based_bwe.h"

#include <rtc_base/logging.h>

namespace xrtc {
    namespace {

        constexpr webrtc::TimeDelta kStreamTimeout = webrtc::TimeDelta::Seconds(2);
        constexpr webrtc::TimeDelta kSendTimeGroupLength = webrtc::TimeDelta::Millis(5);

    } // namespace

    DelayBasedBwe::DelayBasedBwe() :
            video_delay_detector_(std::make_unique<TrendlineEstimator>())
    {
    }

    DelayBasedBwe::~DelayBasedBwe() {
    }

    DelayBasedBwe::Result DelayBasedBwe::IncomingPacketFeedbackVector(
            const webrtc::TransportPacketsFeedback& msg,
            absl::optional<webrtc::DataRate> acked_bitrate)
    {
        // 数据包按照到达时间进行排序
        auto packet_feedback_vector = msg.SortedByReceiveTime();
        if (packet_feedback_vector.empty()) {
            return DelayBasedBwe::Result();
        }

        bool recover_from_overusing = false;
        webrtc::BandwidthUsage prev_detector_state = video_delay_detector_->State();
        for (const auto& packet_feedback : packet_feedback_vector) {
            IncomingPacketFeedback(packet_feedback, msg.feedback_time);
            if (prev_detector_state == webrtc::BandwidthUsage::kBwUnderusing &&
                video_delay_detector_->State() == webrtc::BandwidthUsage::kBwNormal)
            {
                recover_from_overusing = true;
            }
            prev_detector_state = video_delay_detector_->State();
        }

        return MaybeUpdateEstimate(acked_bitrate, recover_from_overusing, msg.feedback_time);
    }

    void DelayBasedBwe::OnRttUpdate(int64_t rtt_ms) {
        rate_control_.SetRtt(webrtc::TimeDelta::Millis(rtt_ms));
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
            video_delay_detector_.reset(new TrendlineEstimator());
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

        /*
        if (calculated_delta) {
            RTC_LOG(LS_WARNING) << "**************send_delta: " << send_time_delta.ms()
                << ", recv_delta: " << recv_time_delta.ms()
                << ", packet_size_delta: " << packet_size_delta;
        }
        */

        video_delay_detector_->Update(recv_time_delta, send_time_delta,
                                      packet_feedback.sent_packet.send_time,
                                      packet_feedback.receive_time,
                                      packet_size,
                                      calculated_delta);
    }

    DelayBasedBwe::Result DelayBasedBwe::MaybeUpdateEstimate(
            absl::optional<webrtc::DataRate> acked_bitrate,
            bool recover_from_overusing,
            webrtc::Timestamp at_time)
    {
        // 根据网络检测状态来动态的调整发送码率
        Result result;
        // 当网络出现过载的时候
        if (video_delay_detector_->State() == webrtc::BandwidthUsage::kBwOverusing) {
            // 已知吞吐量时
            if (acked_bitrate && rate_control_.TimeToReduceFurther(at_time, *acked_bitrate)) {

            }
                // 当我们不知道吞吐量的时候
            else if (!acked_bitrate && rate_control_.ValidEstimate() &&
                     rate_control_.InitialTimeToReduceFurther(at_time))
            {

            }
        }
            // 网络没有出现过载
        else {

        }

        return result;
    }

} // namespace xrtc