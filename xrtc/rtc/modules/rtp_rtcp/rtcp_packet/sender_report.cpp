//
// Created by faker on 2023/6/25.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/sender_report.h"

#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {
    namespace rtcp {

        size_t SenderReport::BlockLength() const {
            return kHeaderSize + kSenderBaseLength +
                   report_blocks_.size() * ReportBlock::kLength;
        }

        // index 可能不是0 packet里边可能有rtcp包了
        bool SenderReport::Create(uint8_t* packet,
                                  size_t* index,
                                  size_t max_length,
                                  PacketReadyCallback callback) const
        {
            while (*index + BlockLength() > max_length) { // 容量已经不够了
                // 如果是第一个包，容量就不足，直接return false
                // 如果是非第一个包，容量不足，可以先将之前已经构建好的包，我们先返回
                if (!OnBufferFull(packet, index, callback)) {
                    return false;
                }
            }

            // 创建头部
            CreateHeader(report_blocks_.size(), kPacketType, HeaderLength(),
                         packet, index);
            // 创建Sender Report固定的部分
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 0], sender_ssrc());
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 4], ntp_time_.seconds());
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 8], ntp_time_.fractions());
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 12], rtp_timestamp_);
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 16], send_packet_count_);
            webrtc::ByteWriter<uint32_t>::WriteBigEndian(
                    &packet[*index + 20], send_packet_octet_);

            *index += kSenderBaseLength;

            // TODO: 写入report block
            return true;
        }

    } // namespace rtcp
} // namespace xrtc
