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
            const size_t kChunkSizeBytes = 2;

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

            if (0 == status_count) {
                RTC_LOG(LS_WARNING) << "Empty transport feedback message not allowd";
                return false;
            }

            // 数据块的起始位置
            size_t index = 16;
            // 数据块的结束位置
            size_t end_index = packet.payload_size();

            // 定义一个vector来保存所有的rtp包状态
            std::vector<uint8_t> delta_sizes;
            delta_sizes.reserve(status_count);

            // 读取完所有的RTP包的状态信息，才会结束循环
            while (delta_sizes.size() < status_count) {
                if (index + kChunkSizeBytes > end_index) {
                    RTC_LOG(LS_WARNING) << "Buffer overlow when parsing packet";
                    return false;
                }

                // 读取一个chunk进行处理
                uint16_t chunk = webrtc::ByteReader<uint16_t>::ReadBigEndian(&payload[index]);
                // 偏移指向下一个chunk
                index += kChunkSizeBytes;

                // 解码chunk
                last_chunk_.Decode(chunk, status_count - delta_sizes.size());
            }

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

        void TransportFeedback::LastChunk::Decode(uint16_t chunk,
                                                  size_t max_size)
        {
            if (0 == (chunk & 0x8000)) { // run length编码块
                DecodeRunLength(chunk, max_size);
            }
            else {

            }
        }

        void TransportFeedback::LastChunk::DecodeRunLength(uint16_t chunk,
                                                           size_t max_size)
        {
            size_ = std::min<size_t>(chunk & 0x1FFF, max_size);
            all_same_ = true;
            // RTP包的状态值
            uint8_t delta_size = (chunk >> 13) & 0x3;
            has_large_data_ = (delta_size > kLarge);
            delta_sizes_[0] = delta_size;
        }

    } // namespace rtcp
} // namespace xrtc