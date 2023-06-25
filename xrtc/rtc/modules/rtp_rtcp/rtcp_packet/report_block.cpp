//
// Created by faker on 2023/6/25.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/report_block.h"

#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {
    namespace rtcp {

// From RFC 3550, RTP: A Transport Protocol for Real-Time Applications.
//
// RTCP report block (RFC 3550).
//
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  0 |                 SSRC_1 (SSRC of first source)                 |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 | fraction lost |       cumulative number of packets lost       |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |           extended highest sequence number received           |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |                      interarrival jitter                      |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |                         last SR (LSR)                         |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 20 |                   delay since last SR (DLSR)                  |
// 24 +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
        bool ReportBlock::Parse(const uint8_t* buffer, size_t len) {
            if (len < kLength) {
                return false;
            }

            source_ssrc_ = webrtc::ByteReader<uint32_t>::ReadBigEndian(&buffer[0]);
            fraction_lost_ = buffer[4];
            cumulative_packets_lost_ =
                    webrtc::ByteReader<int32_t, 3>::ReadBigEndian(&buffer[5]);
            exetened_highest_sequence_number_ =
                    webrtc::ByteReader<uint32_t>::ReadBigEndian(&buffer[8]);
            jitter_ = webrtc::ByteReader<uint32_t>::ReadBigEndian(&buffer[12]);
            last_sr_ = webrtc::ByteReader<uint32_t>::ReadBigEndian(&buffer[16]);
            delay_since_last_sr_ =
                    webrtc::ByteReader<uint32_t>::ReadBigEndian(&buffer[20]);

            return true;
        }

    } // namespace rtcp
} // namespace xrtc

