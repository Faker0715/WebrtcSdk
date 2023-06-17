//
// Created by faker on 2023/6/18.
//

#include "peer_connection.h"
#include <string>
namespace xrtc{

    PeerConnection::PeerConnection() {

    }

    PeerConnection::~PeerConnection() {

    }

    int PeerConnection::SetRemoteSDP(const std::string &sdp) {
        return 0;
    }

    std::string PeerConnection::CreateAnswer(const RTCOfferAnswerOptions &options, const std::string &stream_id) {
        return nullptr;
    }
}