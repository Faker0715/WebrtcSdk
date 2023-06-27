//
// Created by faker on 2023/6/27.
//

#ifndef XRTCSDK_RTP_UTILS_H
#define XRTCSDK_RTP_UTILS_H


#include <api/array_view.h>
#include <system_wrappers/include/ntp_time.h>

namespace xrtc {

    enum class RtpPacketType {
        kRtp = 0,
        kRtcp = 1,
        kUnknown = 2,
    };

    bool IsRtcpPacket(rtc::ArrayView<const uint8_t> packet);
    bool IsRtpPacket(rtc::ArrayView<const uint8_t> packet);
    RtpPacketType InferRtpPacketType(rtc::ArrayView<const uint8_t> packet);

    inline uint32_t CompactNtp(webrtc::NtpTime ntp_time) {
        return (ntp_time.seconds()) << 16 | (ntp_time.fractions() >> 16);
    }

    int64_t CompactNtpRttToMs(uint32_t compact_ntp_interval);

} // namespace xrtc

#endif //XRTCSDK_RTP_UTILS_H
