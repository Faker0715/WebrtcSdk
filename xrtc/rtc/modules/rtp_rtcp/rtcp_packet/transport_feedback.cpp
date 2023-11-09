//
// Created by faker on 2023/11/8.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"

#include <absl/algorithm/container.h>
#include <rtc_base/logging.h>
#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {
    namespace rtcp {
        namespace {

            const size_t kRtcpTransportFeedbackHeaderSize = 4 + 8 + 8;

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

            Clear();

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
                    Clear();
                    return false;
                }

                // 读取一个chunk进行处理
                uint16_t chunk = webrtc::ByteReader<uint16_t>::ReadBigEndian(&payload[index]);
                // 偏移指向下一个chunk
                index += kChunkSizeBytes;

                // 解码chunk
                last_chunk_.Decode(chunk, status_count - delta_sizes.size());
                last_chunk_.AppendTo(&delta_sizes);
            }

            uint16_t seq_no = base_seq_no_;

            // 0, RTP包没有收到，就没有对应的recv_delta
            // 1, RTP包收到了，数据包间隔比较小，recv_delta使用1字节来表示时间
            // 2, RTP包收到了，数据包间隔比较大，recv_delta使用2字节来表示时间
            size_t recv_delta_size = absl::c_accumulate(delta_sizes, 0);

            if (end_index >= index + recv_delta_size) { // 表示存在recv_delta数据块
                for (size_t delta_size : delta_sizes) {
                    if (index + delta_size > end_index) {
                        RTC_LOG(LS_WARNING) << "Buffer overflow when parsing packet";
                        Clear();
                        return false;
                    }

                    switch (delta_size) {
                        case 0:
                            if (include_lost_) {
                                all_packets_.emplace_back(seq_no);
                            }
                            break;
                        case 1: {
                            int16_t delta = payload[index];
                            received_packets_.emplace_back(seq_no, delta);
                            if (include_lost_) {
                                all_packets_.emplace_back(seq_no, delta);
                            }
                            index += delta_size;
                            break;
                        }
                        case 2: {
                            int16_t delta = webrtc::ByteReader<int16_t>::ReadBigEndian(
                                    &payload[index]);
                            received_packets_.emplace_back(seq_no, delta);
                            if (include_lost_) {
                                all_packets_.emplace_back(seq_no, delta);
                            }
                            index += delta_size;
                            break;
                        }
                        case 3:
                            RTC_LOG(LS_WARNING) << "invalid delta size for seq_no: " << seq_no;
                            Clear();
                            return false;
                        default:
                            break;
                    }

                    ++seq_no;
                }
            }
            else { // 不包含recv_delta数据块
                include_timestamps_ = false;
                for (size_t delta_size : delta_sizes) {
                    if (delta_size > 0) {
                        received_packets_.emplace_back(seq_no, 0);
                    }

                    if (include_lost_) {
                        if (delta_size > 0) {
                            // 数据包收到了，但是不包含时间信息
                            all_packets_.emplace_back(seq_no, 0);
                        }
                        else {
                            // 数据包没有收到
                            all_packets_.emplace_back(seq_no);
                        }
                    }

                    ++seq_no;
                }
            }

            size_bytes_ = RtcpPacket::kHeaderSize + index;
            return true;
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

        void TransportFeedback::Clear() {
            last_chunk_.Clear();
            all_packets_.clear();
            received_packets_.clear();
            size_bytes_ = kRtcpTransportFeedbackHeaderSize;
        }

        void TransportFeedback::LastChunk::Decode(uint16_t chunk,
                                                  size_t max_size)
        {
            if (0 == (chunk & 0x8000)) { // run length编码块
                DecodeRunLength(chunk, max_size);
            }
            else if (0 == (chunk & 0x4000)) { // 1bit状态矢量编码
                DecodeOneBit(chunk, max_size);
            }
            else {
                DecodeTwoBit(chunk, max_size);
            }
        }

        void TransportFeedback::LastChunk::AppendTo(std::vector<uint8_t>* deltas) {
            if (all_same_) {
                deltas->insert(deltas->end(), size_, delta_sizes_[0]);
            }
            else {
                deltas->insert(deltas->end(), delta_sizes_, delta_sizes_ + size_);
            }
        }

        void TransportFeedback::LastChunk::Clear() {
            size_ = 0;
            all_same_ = true;
            has_large_delta_ = false;
        }

        void TransportFeedback::LastChunk::DecodeRunLength(uint16_t chunk,
                                                           size_t max_size)
        {
            size_ = std::min<size_t>(chunk & 0x1FFF, max_size);
            all_same_ = true;
            // RTP包的状态值
            uint8_t delta_size = (chunk >> 13) & 0x3;
            has_large_delta_ = (delta_size > kLarge);
            delta_sizes_[0] = delta_size;
        }

        void TransportFeedback::LastChunk::DecodeOneBit(uint16_t chunk,
                                                        size_t max_size)
        {
            size_ = std::min(kOneBitCapacity, max_size);
            has_large_delta_ = false;
            all_same_ = false;
            for (size_t i = 0; i < size_; ++i) {
                delta_sizes_[i] = (chunk >> (kOneBitCapacity - 1 - i)) & 0x01;
            }
        }

        void TransportFeedback::LastChunk::DecodeTwoBit(uint16_t chunk,
                                                        size_t max_size)
        {
            size_ = std::min(kTwoBitCapacity, max_size);
            has_large_delta_ = true;
            all_same_ = false;
            for (size_t i = 0; i < size_; ++i) {
                delta_sizes_[i] = (chunk >> 2 * (kTwoBitCapacity - 1 - i)) & 0x03;
            }
        }

    } // namespace rtcp
} // namespace xrtc