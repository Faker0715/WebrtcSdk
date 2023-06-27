//
// Created by faker on 2023/6/27.
//

#ifndef XRTCSDK_RECEIVER_REPORT_H
#define XRTCSDK_RECEIVER_REPORT_H


#include <vector>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/report_block.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"

namespace xrtc {
    namespace rtcp {

        class ReceiverReport : public RtcpPacket {
        public:
            static const uint8_t kPacketType = 201;

            ReceiverReport() = default;
            ~ReceiverReport() override = default;

            size_t BlockLength() const override;

            bool Create(uint8_t* packet,
                        size_t* index,
                        size_t max_length,
                        PacketReadyCallback callback) const override;

            bool Parse(const CommonHeader& packet);

            const std::vector<ReportBlock>& report_blocks() const {
                return report_blocks_;
            }

        private:
            static const size_t kRrBaseLength = 4;
            std::vector<ReportBlock> report_blocks_;
        };

    } // namespace rtcp
} // namespace xrtc

#endif //XRTCSDK_RECEIVER_REPORT_H
