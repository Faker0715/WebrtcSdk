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
    };

} // namespace xrtc
#endif //XRTCSDK_NETWORK_CONTROLLER_H
