//
// Created by faker on 2023/6/30.
//

#ifndef XRTCSDK_AUDIO_SEND_STREAM_CONFIG_H
#define XRTCSDK_AUDIO_SEND_STREAM_CONFIG_H


#include <stdint.h>

namespace xrtc {

    class RtpRtcpModuleObserver;

    struct AudioSendStreamConfig {
        struct Rtp {
            uint32_t ssrc = 0;
            int payload_type = -1;
            int clock_rate = 48000;
        } rtp;

        // 音频的rtcp包发送间隔
        int rtcp_report_interval_ms = 5000;
        RtpRtcpModuleObserver* rtp_rtcp_module_observer = nullptr;
    };

} // namespace xrtc



#endif //XRTCSDK_AUDIO_SEND_STREAM_CONFIG_H
