//
// Created by faker on 2023/6/30.
//

#include "xrtc/rtc/audio/audio_send_stream.h"

namespace xrtc {

    std::unique_ptr<ModuleRtpRtcpImpl> CreateRtpRtcpModule(webrtc::Clock* clock,
                                                           const AudioSendStreamConfig& asconfig) {
        RtpRtcpInterface::Configuration config;
        config.audio = true;
        config.receiver_only = false;
        config.clock = clock;
        config.local_media_ssrc = asconfig.rtp.ssrc;
        config.payload_type = asconfig.rtp.payload_type;
        config.rtcp_report_interval_ms = asconfig.rtcp_report_interval_ms;
        config.clock_rate = asconfig.rtp.clock_rate;
        config.rtp_rtcp_module_observer = asconfig.rtp_rtcp_module_observer;

        auto rtp_rtcp = std::make_unique<ModuleRtpRtcpImpl>(config);
        return std::move(rtp_rtcp);
    }

    AudioSendStream::AudioSendStream(webrtc::Clock* clock,
                                     const AudioSendStreamConfig& config) :
            config_(config),
            rtp_rtcp_(CreateRtpRtcpModule(clock, config))
    {
        // 设置RTCP包为复合包模式，同时启动定时器
        rtp_rtcp_->SetRTCPStatus(webrtc::RtcpMode::kCompound);
        rtp_rtcp_->SetSendingStatus(true);
    }

    AudioSendStream::~AudioSendStream() {
    }

    void AudioSendStream::UpdateRtpStats(std::shared_ptr<RtpPacketToSend> packet,
                                         bool is_retransmit)
    {
        rtp_rtcp_->UpdateRtpStats(packet, false, is_retransmit);
    }

    void AudioSendStream::OnSendingRtpFrame(uint32_t rtp_timestamp,
                                            int64_t capture_time_ms)
    {
        rtp_rtcp_->OnSendingRtpFrame(rtp_timestamp, capture_time_ms,
                                     false);
    }



} // namespace xrtc

