//
// Created by faker on 2023/6/18.
//

#ifndef XRTCSDK_PEER_CONNECTION_H
#define XRTCSDK_PEER_CONNECTION_H


#include <string>
#include <memory>

#include <system_wrappers/include/clock.h>

//#include "xrtc/media/base/media_frame.h"
#include "xrtc/rtc/pc/session_description.h"
#include "transport_controller.h"
#include "xrtc/rtc/pc/transport_controller.h"
//#include "xrtc/rtc/pc/peer_connection_def.h"
//#include "xrtc/rtc/video/video_send_stream.h"
//#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"

namespace xrtc {

    struct RTCOfferAnswerOptions {
        bool send_audio = true;
        bool send_video = true;
        bool recv_audio = true;
        bool recv_video = true;
        bool use_rtp_mux = true;
        bool use_rtcp_mux = true;
    };

    class PeerConnection{
    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);
        std::string CreateAnswer(const RTCOfferAnswerOptions& options,
                                 const std::string& stream_id);

    private:
        std::unique_ptr<SessionDescription> remote_desc_;
        std::unique_ptr<SessionDescription> local_desc_;
        std::unique_ptr<TransportController> transport_controller_;

        uint32_t local_audio_ssrc_ = 0;
        uint32_t local_video_ssrc_ = 0;
        uint32_t local_video_rtx_ssrc_ = 0;
        uint8_t video_pt_ = 0;
        uint8_t video_rtx_pt_ = 0;

        // 按照规范该值的初始值需要随机
        uint16_t video_seq_ = 1000;

        webrtc::Clock* clock_;
    };

}

#endif //XRTCSDK_PEER_CONNECTION_H
