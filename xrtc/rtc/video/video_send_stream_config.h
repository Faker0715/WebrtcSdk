//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_VIDEO_SEND_STREAM_CONFIG_H
#define XRTCSDK_VIDEO_SEND_STREAM_CONFIG_H


#include <stdint.h>

namespace xrtc {

    class RtpRtcpModuleObserver;
    class TransportFeedbackObserver;

    struct VideoSendStreamConfig {
        struct Rtp {
            uint32_t ssrc = 0;
            int payload_type = -1;
            int clock_rate = 90000;

            struct Rtx {
                uint32_t ssrc = 0;
                int payload_type = -1;
            } rtx;

        } rtp;

        // 视频的rtcp包发送间隔
        int rtcp_report_interval_ms = 1000;
        RtpRtcpModuleObserver* rtp_rtcp_module_observer = nullptr;
        TransportFeedbackObserver* transport_feedback_observer = nullptr;
    };

} // namespace xrtc

#endif //XRTCSDK_VIDEO_SEND_STREAM_CONFIG_H
