//
// Created by faker on 2023/11/8.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"

#include <rtc_base/logging.h>
#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {
    namespace rtcp {
        namespace {

// 通用的rtcp部分，8字节
// transport feedback头部，8字节
// 至少需要包含一个packet chunk, 2字节
            const size_t kMinPayloadSizeBytes = 8 + 8 + 2;

//    Message format
//
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |V=2|P|  FMT=15 |    PT=205     |           length              |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  0 |                     SSRC of packet sender                     |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 |                      SSRC of media source                     |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |      base sequence number     |      packet status count      |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |                 reference time                | fb pkt. count |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |          packet chunk         |         packet chunk          |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    .                                                               .
//    .                                                               .
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |         packet chunk          |  recv delta   |  recv delta   |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    .                                                               .
//    .                                                               .
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |           recv delta          |  recv delta   | zero padding  |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        } // namespace

        bool TransportFeedback::Parse(const rtcp::CommonHeader& packet) {
            // 检查长度是否满足最低要求
            if (packet.payload_size() < kMinPayloadSizeBytes) {
                RTC_LOG(LS_WARNING) << "Buffer is too small for a transport feedback "
                                    << "packet, payload_size: " << packet.payload_size()
                                    << ", min payload_size: " << kMinPayloadSizeBytes;
                return false;
            }

            const uint8_t* payload = packet.payload();
            // 解析rtcp通用部分
            ParseCommonFeedback(payload);

            // 解析transport feedback头部
            base_seq_no_ = webrtc::ByteReader<uint16_t>::ReadBigEndian(&payload[8]);
            uint16_t status_count = webrtc::ByteReader<uint16_t>::ReadBigEndian(&payload[10]);
            base_time_ticks_ = webrtc::ByteReader<int32_t, 3>::ReadBigEndian(&payload[12]);
            feedback_seq_ = payload[15];

            return false;
        }

        size_t TransportFeedback::BlockLength() const {
            return 0;
        }

        bool TransportFeedback::Create(uint8_t* packet,
                                       size_t* index,
                                       size_t max_length,
                                       PacketReadyCallback callback) const
        {
            return false;
        }

    } // namespace rtcp
} // namespace xrtc