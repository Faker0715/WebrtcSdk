//
// Created by faker on 2023/6/27.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtp_utils.h"

#include <rtc_base/numerics/divide_round.h>

namespace xrtc {
    namespace {

        const size_t kMinRtpPacketLength = 12;
        const size_t kMinRtcpPacketLength = 4;
        const uint8_t kRtpVersion = 2;

        bool HasCorrectVersion(rtc::ArrayView<const uint8_t> packet) {
            return (packet[0] >> 6) == kRtpVersion;
        }

        bool PayloadTypeIsReservedForRtcp(uint8_t pt) {
            return 64 <= pt && pt < 96;
        }

    } // namespace


    bool IsRtcpPacket(rtc::ArrayView<const uint8_t> packet) {
        return packet.size() > kMinRtcpPacketLength && HasCorrectVersion(packet)
               && PayloadTypeIsReservedForRtcp(packet[1] & 0x7F);
    }

    bool IsRtpPacket(rtc::ArrayView<const uint8_t> packet) {
        return packet.size() > kMinRtpPacketLength && HasCorrectVersion(packet)
               && !PayloadTypeIsReservedForRtcp(packet[1] & 0x7F);
    }

    RtpPacketType InferRtpPacketType(rtc::ArrayView<const uint8_t> packet) {
        if (IsRtcpPacket(packet)) {
            return RtpPacketType::kRtcp;
        }
        else if (IsRtpPacket(packet)) {
            return RtpPacketType::kRtp;
        }
        return RtpPacketType::kUnknown;
    }

    int64_t CompactNtpRttToMs(uint32_t compact_ntp_interval) {
        if (compact_ntp_interval > 0x80000000) {
            return 1;
        }

        int64_t value = static_cast<int64_t>(compact_ntp_interval);
        int64_t ms = webrtc::DivideRoundToNearest(value * 1000, 1 << 16);
        return ms < 1 ? 1 : ms;
    }

} // namespace xrtc

