//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_PACKET_TO_SEND_H
#define XRTCSDK_RTP_PACKET_TO_SEND_H


#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"

namespace xrtc {

    class RtpPacketToSend : public RtpPacket {
    public:
        RtpPacketToSend();
        RtpPacketToSend(size_t capacity);

    };

} // namespace xrtc

#endif //XRTCSDK_RTP_PACKET_TO_SEND_H
