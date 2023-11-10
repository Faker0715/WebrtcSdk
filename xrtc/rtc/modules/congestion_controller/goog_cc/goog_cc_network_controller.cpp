//
// Created by faker on 2023/11/10.
//
#include "xrtc/rtc/modules/congestion_controller/goog_cc/goog_cc_network_controller.h"

namespace xrtc {

    GoogCcNetworkController::GoogCcNetworkController() {
    }

    GoogCcNetworkController::~GoogCcNetworkController() {
    }

    webrtc::NetworkControlUpdate GoogCcNetworkController::OnTransportPacketsFeedback(const webrtc::TransportPacketsFeedback& report) {
        return webrtc::NetworkControlUpdate();
    }

} // namespace xrtc
