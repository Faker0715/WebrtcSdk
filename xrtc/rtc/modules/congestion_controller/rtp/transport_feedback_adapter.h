#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_

#include <api/units/time_delta.h>
#include <api/transport/network_types.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"

namespace xrtc {

class TransportFeedbackAdapter {
public:
    TransportFeedbackAdapter();
    ~TransportFeedbackAdapter();

    absl::optional<webrtc::TransportPacketsFeedback> ProcessTransportFeedback(
        const rtcp::TransportFeedback& feedback,
        webrtc::Timestamp feedback_time);

private:
    std::vector<webrtc::PacketResult> ProcessTransportFeedbackInner(
        const rtcp::TransportFeedback& feedback,
        webrtc::Timestamp feedback_time);

private:
    webrtc::Timestamp current_offset_ = webrtc::Timestamp::MinusInfinity();
    webrtc::TimeDelta last_timestamp_ = webrtc::TimeDelta::MinusInfinity();
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_