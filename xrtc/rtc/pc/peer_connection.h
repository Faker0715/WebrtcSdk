//
// Created by faker on 2023/6/18.
//

#ifndef XRTCSDK_PEER_CONNECTION_H
#define XRTCSDK_PEER_CONNECTION_H


#include <string>
#include <memory>

#include <system_wrappers/include/clock.h>

#include "xrtc/media/base/media_frame.h"
#include "xrtc/rtc/pc/session_description.h"
#include "transport_controller.h"
#include "xrtc/rtc/pc/transport_controller.h"
#include "peer_connection_def.h"
#include "xrtc/rtc/pc/peer_connection_def.h"
#include "xrtc/rtc/video/video_send_stream.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"

namespace xrtc {

    struct RTCOfferAnswerOptions {
        bool send_audio = true;
        bool send_video = true;
        bool recv_audio = true;
        bool recv_video = true;
        bool use_rtp_mux = true;
        bool use_rtcp_mux = true;
    };

    class PeerConnection : public sigslot::has_slots<>,
                           public RtpRtcpModuleObserver
    {

    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);
        std::string CreateAnswer(const RTCOfferAnswerOptions& options,
                                 const std::string& stream_id);
        bool SendEncodedImage(std::shared_ptr<MediaFrame> frame);

        // RtpRtcpModuleObserver
        void OnLocalRtcpPacket(webrtc::MediaType media_type,
                               const uint8_t* data, size_t len) override;
        void OnNetworkInfo(int64_t rtt_ms, int32_t packets_lost,
                           uint8_t fraction_lost, uint32_t jitter) override;
        void OnNackReceived(webrtc::MediaType media_type,
                            const std::vector<uint16_t>& nack_list) override;

        sigslot::signal2<PeerConnection*, PeerConnectionState> SignalConnectionState;
        sigslot::signal5<PeerConnection*, int64_t, int32_t, uint8_t, uint32_t>
                SignalNetworkInfo;

    private:
        void OnIceState(TransportController*, ice::IceTransportState ice_state);
        void OnRtcpPacketReceived(TransportController*, const char* data,
                                  size_t len, int64_t);
        void CreateVideoSendStream(VideoContentDescription* video_content);
//        void AddVideoCache(std::shared_ptr<RtpPacketToSend> packet);
//        std::shared_ptr<RtpPacketToSend> FindVideoCache(uint16_t seq);
    private:
        std::unique_ptr<SessionDescription> remote_desc_;
        std::unique_ptr<SessionDescription> local_desc_;
        std::unique_ptr<TransportController> transport_controller_;

        uint32_t local_audio_ssrc_ = 0;
        uint32_t local_video_ssrc_ = 0;
        uint32_t local_video_rtx_ssrc_ = 0;
        uint8_t video_pt_ = 0;
        uint8_t video_rtx_pt_ = 0;

        // 规范是随机 这里先不随机
        uint16_t video_seq_ = 1000;

        PeerConnectionState pc_state_ = PeerConnectionState::kNew;
        webrtc::Clock* clock_;
        VideoSendStream* video_send_stream_ = nullptr;
//        std::vector<std::shared_ptr<RtpPacketToSend>> video_cache_;
    };

}

#endif //XRTCSDK_PEER_CONNECTION_H
