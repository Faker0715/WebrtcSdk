//
// Created by faker on 2023/6/18.
//

#ifndef XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_
#define XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_

#include <string>
#include <memory>

#include <system_wrappers/include/clock.h>
#include <api/task_queue/task_queue_factory.h>

#include "xrtc/media/base/media_frame.h"
#include "xrtc/rtc/pc/session_description.h"
#include "xrtc/rtc/pc/transport_controller.h"
#include "xrtc/rtc/pc/peer_connection_def.h"
#include "xrtc/rtc/pc/rtp_transport_controller_send.h"
#include "xrtc/rtc/audio/audio_send_stream.h"
#include "xrtc/rtc/video/video_send_stream.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_header_extension_map.h"

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
                           public RtpRtcpModuleObserver,
                           public PacingController::PacketSender
    {
    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);
        std::string CreateAnswer(const RTCOfferAnswerOptions& options,
                                 const std::string& stream_id);
        bool SendEncodedAudio(std::shared_ptr<MediaFrame> frame);
        bool SendEncodedImage(std::shared_ptr<MediaFrame> frame);

        // RtpRtcpModuleObserver
        void OnLocalRtcpPacket(webrtc::MediaType media_type,
                               const uint8_t* data, size_t len) override;
        void OnNetworkInfo(int64_t rtt_ms, int32_t packets_lost,
                           uint8_t fraction_lost, uint32_t jitter) override;
        void OnNackReceived(webrtc::MediaType media_type,
                            const std::vector<uint16_t>& nack_list) override;

        // PacingController::PacketSender
        void SendPacket(std::unique_ptr<RtpPacketToSend> packet) override;

        sigslot::signal2<PeerConnection*, PeerConnectionState> SignalConnectionState;
        sigslot::signal5<PeerConnection*, int64_t, int32_t, uint8_t, uint32_t>
                SignalNetworkInfo;

    private:
        void OnIceState(TransportController*, ice::IceTransportState ice_state);
        void OnRtcpPacketReceived(TransportController*, const char* data,
                                  size_t len, int64_t);
        void CreateAudioSendStream(AudioContentDescription* audio_content);
        void CreateVideoSendStream(VideoContentDescription* video_content);
        void AddVideoCache(std::shared_ptr<RtpPacketToSend> packet);
        std::shared_ptr<RtpPacketToSend> FindVideoCache(uint16_t seq);
        void AddPacketToTransportFeedback(uint16_t packet_id,
                                          RtpPacketToSend* packet);

    private:
        std::unique_ptr<SessionDescription> remote_desc_;
        std::unique_ptr<SessionDescription> local_desc_;
        std::unique_ptr<TransportController> transport_controller_;

        uint32_t local_audio_ssrc_ = 0;
        uint32_t local_video_ssrc_ = 0;
        uint32_t local_video_rtx_ssrc_ = 0;
        uint32_t audio_pt_ = 0;
        uint8_t video_pt_ = 0;
        uint8_t video_rtx_pt_ = 0;

        // 按照规范该值的初始值需要随机
        uint16_t audio_seq_ = 1000;
        uint16_t video_seq_ = 1000;
        uint16_t transport_seq_ = 1000;

        PeerConnectionState pc_state_ = PeerConnectionState::kNew;
        webrtc::Clock* clock_;
        AudioSendStream* audio_send_stream_ = nullptr;
        VideoSendStream* video_send_stream_ = nullptr;
        std::vector<std::shared_ptr<RtpPacketToSend>> video_cache_;
        std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
        std::unique_ptr<RtpTransportControllerSend> transport_send_;
        RtpHeaderExtensionMap rtp_header_extension_map_;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_