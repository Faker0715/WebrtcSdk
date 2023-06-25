//
// Created by faker on 2023/6/25.
//

#ifndef XRTCSDK_SENDER_REPORT_H
#define XRTCSDK_SENDER_REPORT_H


#include <vector>

#include <system_wrappers/include/ntp_time.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/report_block.h"

namespace xrtc {
    namespace rtcp {

        class SenderReport : public RtcpPacket {
        public:
            static const uint8_t kPacketType = 200;

            SenderReport() = default;
            ~SenderReport() override = default;

            void SetNtpTime(webrtc::NtpTime ntp_time) {
                ntp_time_ = ntp_time;
            }
            webrtc::NtpTime ntp_time() { return ntp_time_; }

            void SetRtpTimestamp(uint32_t rtp_timestamp) {
                rtp_timestamp_ = rtp_timestamp;
            }
            uint32_t rtp_timestamp() { return rtp_timestamp_; }

            void SetSendPacketCount(uint32_t packet_count) {
                send_packet_count_ = packet_count;
            }
            uint32_t send_packet_count() { return send_packet_count_; }

            void SetSendPacketOctet(uint32_t packet_octet) {
                send_packet_octet_ = packet_octet;
            }
            uint32_t send_packet_octet() { return send_packet_octet_; }

            size_t BlockLength() const override;
            bool Create(uint8_t* packet,
                        size_t* index,
                        size_t max_length,
                        PacketReadyCallback callback) const override;

        private:
            static const size_t kSenderBaseLength = 24;

            webrtc::NtpTime ntp_time_;
            uint32_t rtp_timestamp_;
            uint32_t send_packet_count_;
            uint32_t send_packet_octet_;
            std::vector<ReportBlock> report_blocks_;
        };

    } // namespace rtcp

} // namespace xrtc

#endif //XRTCSDK_SENDER_REPORT_H
