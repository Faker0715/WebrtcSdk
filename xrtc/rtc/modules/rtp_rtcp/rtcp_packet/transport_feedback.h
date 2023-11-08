//
// Created by faker on 2023/11/8.
//

#ifndef XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_
#define XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"

namespace xrtc {
    namespace rtcp {

        class TransportFeedback : public Rtpfb {
        public:
            static const uint8_t kFeedbackMessageType = 15;

            bool Parse(const rtcp::CommonHeader& packet);

            size_t BlockLength() const override;

            virtual bool Create(uint8_t* packet,
                                size_t* index,
                                size_t max_length,
                                PacketReadyCallback callback) const override;

        private:
            uint16_t base_seq_no_ = 0;
            int32_t base_time_ticks_ = 0;
            uint8_t feedback_seq_ = 0;
        };

    } // namespace rtcp
} // namespace xrtc

#endif // XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_