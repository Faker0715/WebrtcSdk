#include "xrtc/rtc/modules/congestion_controller/rtp/transport_feedback_adapter.h"

#include <rtc_base/logging.h>

namespace xrtc {

TransportFeedbackAdapter::TransportFeedbackAdapter() {
}

TransportFeedbackAdapter::~TransportFeedbackAdapter() {
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
        webrtc::TimeDelta delta = feedback.GetBaseDelta(last_timestamp_.us());
        if (current_offset_ < webrtc::Timestamp::Zero() - delta) {
            RTC_LOG(LS_WARNING) << "Unexpected timestamp in transport feedback packet";
            current_offset_ = feedback_time;
        }
        
        current_offset_ += delta;
    }

    last_timestamp_ = feedback.GetBaseTime();

    return std::vector<webrtc::PacketResult>();
}

} // namespace xrtc