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

        DelayBasedBwe::Result result;
        result = delay_base_bwe_->IncomingPacketFeedbackVector(report);
        return webrtc::NetworkControlUpdate();
    }

} // namespace xrtc