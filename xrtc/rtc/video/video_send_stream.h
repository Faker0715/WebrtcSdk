//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_VIDEO_SEND_STREAM_H
#define XRTCSDK_VIDEO_SEND_STREAM_H


#include <system_wrappers/include/clock.h>

#include "xrtc/rtc/video/video_send_stream_config.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_impl.h"

namespace xrtc {

    class VideoSendStream {
    public:
        VideoSendStream(webrtc::Clock* clock, const VideoSendStreamConfig& config);
        ~VideoSendStream();

//        void UpdateRtpStats(std::shared_ptr<RtpPacketToSend> packet,
//                            bool is_rtx, bool is_retransmit);
        void OnSendingRtpFrame(uint32_t rtp_timestamp,
                               int64_t capture_time_ms,
                               bool forced_report);
        void DeliverRtcp(const uint8_t* packet, size_t length);
//        std::unique_ptr<RtpPacketToSend> BuildRtxPacket(
//                std::shared_ptr<RtpPacketToSend> packet);

    private:
        VideoSendStreamConfig config_;
        std::unique_ptr<ModuleRtpRtcpImpl> rtp_rtcp_;
        uint16_t rtx_seq_ = 1000;
    };

} // namespace xrtc



#endif //XRTCSDK_VIDEO_SEND_STREAM_H
