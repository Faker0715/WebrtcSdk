//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_PACKET_TO_SEND_H
#define XRTCSDK_RTP_PACKET_TO_SEND_H


#include <absl/types/optional.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"

namespace xrtc {

    class RtpPacketToSend : public RtpPacket {
    public:
        RtpPacketToSend();
        RtpPacketToSend(const RtpHeaderExtensionMap* extensions);
        RtpPacketToSend(const RtpHeaderExtensionMap* extensions, size_t capacity);

        void set_packet_type(RtpPacketMediaType type) {
            packet_type_ = type;
        }

        absl::optional<RtpPacketMediaType> packet_type() const {
            return packet_type_;
        }

    private:
        absl::optional<RtpPacketMediaType> packet_type_;
    };

} // namespace xrtc
#endif //XRTCSDK_RTP_PACKET_TO_SEND_H
