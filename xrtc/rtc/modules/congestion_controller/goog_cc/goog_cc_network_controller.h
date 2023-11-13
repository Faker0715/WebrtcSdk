//
// Created by faker on 2023/11/10.
//

#ifndef XRTCSDK_GOOG_CC_NETWORK_CONTROLLER_H
#define XRTCSDK_GOOG_CC_NETWORK_CONTROLLER_H


#include "xrtc/rtc/pc/network_controller.h"
#include "xrtc/rtc/modules/congestion_controller/goog_cc/delay_based_bwe.h"

namespace xrtc {

    class GoogCcNetworkController : public NetworkControllerInterface {
    public:
        GoogCcNetworkController();
        ~GoogCcNetworkController() override;

        webrtc::NetworkControlUpdate OnTransportPacketsFeedback(
                const webrtc::TransportPacketsFeedback& report) override;

    private:
        std::unique_ptr<DelayBasedBwe> delay_base_bwe_;
    };

} // namespace xrtc

#endif //XRTCSDK_GOOG_CC_NETWORK_CONTROLLER_H
