#include "xrtc/rtc/modules/congestion_controller/rtp/transport_feedback_adapter.h"

#include <rtc_base/logging.h>

namespace xrtc {

    constexpr webrtc::TimeDelta kSendTimeHistoryWindow = webrtc::TimeDelta::Seconds(60);

    TransportFeedbackAdapter::TransportFeedbackAdapter() {
    }

    TransportFeedbackAdapter::~TransportFeedbackAdapter() {
    }

    absl::optional<webrtc::SentPacket> TransportFeedbackAdapter::ProcessSentPacket(
            const rtc::SentPacket& sent_packet)
    {
        auto send_time = webrtc::Timestamp::Millis(sent_packet.send_time_ms);
        int64_t unwrapped_seq_num = seq_num_unwrapper_.Unwrap(
                sent_packet.packet_id);
        auto it = history_.find(unwrapped_seq_num);
        if (it != history_.end()) { // 找到了发送记录
            bool packet_retransmit = it->second.sent.send_time.IsFinite();
            it->second.sent.send_time = send_time;
            last_send_time_ = std::max(last_send_time_, send_time);

            if (!packet_retransmit) {
                return it->second.sent;
            }
        }

        return absl::nullopt;
    }

    void TransportFeedbackAdapter::AddPacket(webrtc::Timestamp creation_time,
                                             size_t overhead_bytes,
                                             const RtpPacketSendInfo& send_info)
    {
        PacketFeedback packet;
        packet.creation_time = creation_time;
        packet.sent.sequence_number = seq_num_unwrapper_.Unwrap(send_info.transport_sequence_number);
        packet.sent.size = webrtc::DataSize::Bytes(send_info.length + overhead_bytes);
        packet.sent.audio = (send_info.packet_type == RtpPacketMediaType::kAudio);

        // 清理窗口时间以外的老的数据包
        while (!history_.empty() && packet.creation_time -
                                    history_.begin()->second.creation_time >
                                    kSendTimeHistoryWindow)
        {
            history_.erase(history_.begin());
        }

        history_.insert(std::make_pair(packet.sent.sequence_number,
                                       packet));
    }

    absl::optional<webrtc::TransportPacketsFeedback>
    TransportFeedbackAdapter::ProcessTransportFeedback(
            const rtcp::TransportFeedback& feedback,
            webrtc::Timestamp feedback_time)
    {
        if (0 == feedback.GetPacketStatusCount()) {
            RTC_LOG(LS_WARNING) << "Empty rtp packet in transport feedback";
            return absl::nullopt;
        }

        webrtc::TransportPacketsFeedback msg;
        msg.feedback_time = feedback_time;
        msg.packet_feedbacks = ProcessTransportFeedbackInner(
                feedback, feedback_time);

        return absl::optional<webrtc::TransportPacketsFeedback>();
    }

    std::vector<webrtc::PacketResult>
    TransportFeedbackAdapter::ProcessTransportFeedbackInner(
            const rtcp::TransportFeedback& feedback,
            webrtc::Timestamp feedback_time)
    {
        if (last_timestamp_.IsInfinite()) { // 第一次收到feedback
            current_offset_ = feedback_time;
        }
        else {
            webrtc::TimeDelta delta = feedback.GetBaseDelta(last_timestamp_.us())
                    .RoundDownTo(webrtc::TimeDelta::Millis(1));

            if (current_offset_ < webrtc::Timestamp::Zero() - delta) {
                RTC_LOG(LS_WARNING) << "Unexpected timestamp in transport feedback packet";
                current_offset_ = feedback_time;
            }
            else {
                current_offset_ += delta;
            }
        }

        last_timestamp_ = feedback.GetBaseTime();

        return std::vector<webrtc::PacketResult>();
    }

} // namespace xrtc