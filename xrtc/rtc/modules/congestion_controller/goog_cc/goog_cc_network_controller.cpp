#include "xrtc/rtc/modules/congestion_controller/goog_cc/goog_cc_network_controller.h"

namespace xrtc {

    GoogCcNetworkController::GoogCcNetworkController() :
            delay_base_bwe_(std::make_unique<DelayBasedBwe>())
    {
    }

    GoogCcNetworkController::~GoogCcNetworkController() {
    }

    webrtc::NetworkControlUpdate GoogCcNetworkController::OnTransportPacketsFeedback(
            const webrtc::TransportPacketsFeedback& report)
    {
        if (report.packet_feedbacks.empty()) {
            return webrtc::NetworkControlUpdate();
        }

        absl::optional<webrtc::DataRate> acked_bitrate;
        DelayBasedBwe::Result result;
        result = delay_base_bwe_->IncomingPacketFeedbackVector(report, acked_bitrate);

        return webrtc::NetworkControlUpdate();
    }

    webrtc::NetworkControlUpdate GoogCcNetworkController::OnRttUpdate(int64_t rtt_ms) {
        delay_base_bwe_->OnRttUpdate(rtt_ms);
        return webrtc::NetworkControlUpdate();
    }

} // namespace xrtc