//
// Created by faker on 2023/6/30.
//

#ifndef XRTCSDK_AUDIO_SEND_STREAM_H
#define XRTCSDK_AUDIO_SEND_STREAM_H


#include <system_wrappers/include/clock.h>

#include "xrtc/rtc/audio/audio_send_stream_config.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_impl.h"

namespace xrtc {

    class AudioSendStream {
    public:
        AudioSendStream(webrtc::Clock* clock, const AudioSendStreamConfig& config);
        ~AudioSendStream();

        void UpdateRtpStats(std::shared_ptr<RtpPacketToSend> packet,
                            bool is_retransmit);
        void OnSendingRtpFrame(uint32_t rtp_timestamp,
                               int64_t capture_time_ms);

    private:
        AudioSendStreamConfig config_;
        std::unique_ptr<ModuleRtpRtcpImpl> rtp_rtcp_;
    };

} // namespace xrtc



#endif //XRTCSDK_AUDIO_SEND_STREAM_H
