//
// Created by faker on 2023/6/19.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtp_format.h"

#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"

namespace xrtc {

    std::unique_ptr<RtpPacketizer> RtpPacketizer::Create(webrtc::VideoCodecType type,
                                                         rtc::ArrayView<const uint8_t> payload,
                                                         const RtpPacketizer::Config& config)
    {
        switch (type) {
            case webrtc::kVideoCodecH264:
                return std::make_unique<RtpPacketizerH264>(payload, config);
            default:
                return nullptr;
        }
    }

    std::vector<int> RtpPacketizer::SplitAboutEqual(size_t payload_size,
                                                    const PayloadLimits& limits)
    {
        std::vector<int> result;
        // 容量足够
        if (limits.max_payload_len >= payload_size + limits.single_packet_reduction_len) {
            result.push_back(payload_size);
            return result;
        }

        // 容量太小
        if (limits.max_payload_len - limits.first_packet_reduction_len < 1 ||
            limits.max_payload_len - limits.last_packet_reduction_len < 1)
        {
            return result;
        }

        // 需要均分的总字节数
        size_t total_bytes = payload_size + limits.first_packet_reduction_len
                             + limits.last_packet_reduction_len;
        // 计算出我们应该分配多少个包合适，向上取整
        size_t num_packets_left = (total_bytes + limits.max_payload_len - 1) /
                                  limits.max_payload_len;
        if (num_packets_left == 1) {
            num_packets_left = 2;
        }

        // 计算每一个分配的字节数
        size_t bytes_per_packet = total_bytes / num_packets_left;
        // 计算出有多少个包比其他包多1个字节
        size_t num_larger_packet = total_bytes % num_packets_left;

        int remain_data = payload_size;
        bool first_packet = true;
        while (remain_data > 0) {
            // 剩余的包需要多分配一个字节
            // total_bytes 5
            // 分配的个数3个包
            // 5 / 3 = 1, 1 2 2 11 3
            // 5 % 3 = 2
            if (num_packets_left == num_larger_packet) {
                ++bytes_per_packet;
            }
            int current_packet_bytes = bytes_per_packet;

            // 考虑第一个包的大小
            if (first_packet) {
                if (current_packet_bytes - limits.first_packet_reduction_len > 1) {
                    current_packet_bytes -= limits.first_packet_reduction_len;
                }
                else {
                    current_packet_bytes = 1;
                }
            }

            // 当剩余数据不足时，需要特殊考虑
            if (current_packet_bytes > remain_data) {
                current_packet_bytes = remain_data;
            }

            // 确保最后一个分组能够分到数据
            if (num_packets_left == 2 && current_packet_bytes == remain_data) {
                --current_packet_bytes;
            }

            remain_data -= current_packet_bytes;
            num_packets_left--;
            result.push_back(current_packet_bytes);
            first_packet = false;
        }

        return result;
    }

} // namespace xrtc
