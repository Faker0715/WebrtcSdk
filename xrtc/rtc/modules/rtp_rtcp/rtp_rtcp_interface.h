//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_RTCP_INTERFACE_H
#define XRTCSDK_RTP_RTCP_INTERFACE_H
#include <vector>

#include <system_wrappers/include/clock.h>
#include <api/media_types.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"

namespace xrtc {

    class RtpRtcpModuleObserver {
    public:
        virtual void OnLocalRtcpPacket(webrtc::MediaType media_type,
                                       const uint8_t* data, size_t len) = 0;
        virtual void OnNetworkInfo(int64_t rtt_ms, int32_t packets_lost,
                                   uint8_t fraction_lost, uint32_t jitter) = 0;
        virtual void OnNackReceived(webrtc::MediaType media_type,
                                    const std::vector<uint16_t>& nack_list) = 0;
    };

    class RtpRtcpInterface {
    public:
        struct Configuration {
            bool audio = false;
            bool receiver_only = false;
            webrtc::Clock* clock = nullptr;
            uint32_t local_media_ssrc = 0;
            int payload_type = -1;
            int clock_rate = 0;
            int rtcp_report_interval_ms = 0;
            RtpRtcpModuleObserver* rtp_rtcp_module_observer = nullptr;
            TransportFeedbackObserver* transport_feedback_observer = nullptr;
        };
    };

} // namespace xrtc
#endif //XRTCSDK_RTP_RTCP_INTERFACE_H
