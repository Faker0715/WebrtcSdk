//
// Created by faker on 2023/6/27.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"

#include "modules/rtp_rtcp/source/byte_io.h"

namespace xrtc {
    namespace rtcp {

        void Rtpfb::ParseCommonFeedback(const uint8_t* payload) {
            SetSenderSsrc(webrtc::ByteReader<uint32_t>::ReadBigEndian(payload));
            SetMediaSsrc(webrtc::ByteReader<uint32_t>::ReadBigEndian(payload + 4));
        }

    } // namespace rtcp
} // namespace xrtc
