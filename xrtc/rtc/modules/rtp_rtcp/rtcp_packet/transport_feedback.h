//
// Created by faker on 2023/11/8.
//

#ifndef XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_
#define XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"

namespace xrtc {
    namespace rtcp {

        class TransportFeedback : public Rtpfb {
        public:
            static const uint8_t kFeedbackMessageType = 15;
        };

    } // namespace rtcp
} // namespace xrtc

#endif // XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_