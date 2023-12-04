//
// Created by faker on 2023/11/10.
//

#ifndef XRTCSDK_NETWORK_CONTROLLER_H
#define XRTCSDK_NETWORK_CONTROLLER_H

#include <api/transport/network_types.h>

namespace xrtc {

    class NetworkControllerInterface {
    public:
        virtual ~NetworkControllerInterface() {}

        virtual webrtc::NetworkControlUpdate OnTransportPacketsFeedback(
                const webrtc::TransportPacketsFeedback&) = 0;
        virtual webrtc::NetworkControlUpdate OnRttUpdate(int64_t rtt_ms) = 0;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_PC_NETWORK_CONTROLLER_H_
