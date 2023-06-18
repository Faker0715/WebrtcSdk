//
// Created by faker on 2023/6/18.
//

#ifndef XRTCSDK_PEER_CONNECTION_DEF_H
#define XRTCSDK_PEER_CONNECTION_DEF_H

namespace xrtc {

    enum class PeerConnectionState {
        kNew,
        kConnecting,
        kConnected,
        kDisconnected,
        kFailed,
        kClosed,
    };

}
#endif //XRTCSDK_PEER_CONNECTION_DEF_H
