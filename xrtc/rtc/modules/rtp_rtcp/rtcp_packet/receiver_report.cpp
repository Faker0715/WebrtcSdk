//
// Created by faker on 2023/6/27.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/receiver_report.h"

#include <rtc_base/logging.h>
#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {
    namespace rtcp {

        size_t ReceiverReport::BlockLength() const {
            return kHeaderSize + kRrBaseLength + report_blocks_.size() * ReportBlock::kLength;
        }

        bool ReceiverReport::Create(uint8_t* packet,
                                    size_t* index,
                                    size_t max_length,
                                    PacketReadyCallback callback) const
        {
            return false;
        }

// RTCP receiver report (RFC 3550).
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|    RC   |   PT=RR=201   |             length            |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                     SSRC of packet sender                     |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  |                         report block(s)                       |
//  |                            ....                               |
        bool ReceiverReport::Parse(const CommonHeader& packet) {
            const uint8_t report_count = packet.count();
            if (packet.payload_size() < kRrBaseLength + report_count * ReportBlock::kLength) {
                RTC_LOG(LS_WARNING) << "rr payload_size is not enough, payload_size: "
                                    << packet.payload_size() << ", report_count: " << report_count;
                return false;
            }

            const uint8_t* payload = packet.payload();

            SetSenderSsrc(webrtc::ByteReader<uint32_t>::ReadBigEndian(payload));

            const uint8_t* next_report_block = payload + kRrBaseLength;
            report_blocks_.resize(report_count);
            for (auto& report_block : report_blocks_) {
                report_block.Parse(next_report_block, ReportBlock::kLength);
                next_report_block += ReportBlock::kLength;
            }

            return true;
        }


    } // namespace rtcp
} // namespace xrtc
