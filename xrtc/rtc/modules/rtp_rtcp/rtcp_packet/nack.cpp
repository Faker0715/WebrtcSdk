//
// Created by faker on 2023/6/27.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/nack.h"

#include <rtc_base/logging.h>
#include <modules/rtp_rtcp/source/byte_io.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"

namespace xrtc {
    namespace rtcp {

        size_t Nack::BlockLength() const {
            return size_t();
        }

        bool Nack::Create(uint8_t* packet,
                          size_t* index,
                          size_t max_length,
                          PacketReadyCallback callback) const
        {
            return false;
        }

        bool Nack::Parse(const rtcp::CommonHeader& packet) {
            if (packet.payload_size() < kCommonFeedabackLength
                                        + kNackItemLength)
            {
                RTC_LOG(LS_WARNING) << "payload length " << packet.payload_size()
                                    << " is too small for nack";
                return false;
            }

            // 解析Sender ssrc和media source ssrc
            ParseCommonFeedback(packet.payload());
            // 解析FCI
            size_t nack_items = (packet.payload_size() - kCommonFeedabackLength) /
                                kNackItemLength;
            packet_ids_.clear();
            packed_.resize(nack_items);

            const uint8_t* next_nack = packet.payload() + kCommonFeedabackLength;
            for (size_t i = 0; i < nack_items; ++i) {
                packed_[i].first_pid = webrtc::ByteReader<uint16_t>::ReadBigEndian(next_nack);
                packed_[i].bitmask = webrtc::ByteReader<uint16_t>::ReadBigEndian(next_nack + 2);
                next_nack += kNackItemLength;
            }

            Unpack();

            return true;
        }

        void Nack::Unpack() {
            for (const PackedNack& nack : packed_) {
                packet_ids_.push_back(nack.first_pid);
                // 循环判断bitmask
                uint16_t pid = nack.first_pid + 1;
                for (uint16_t nack_item = nack.bitmask; nack_item != 0; nack_item >>= 1, ++pid) {
                    if (nack_item & 1) {
                        packet_ids_.push_back(pid);
                    }
                }
            }
        }

    } // namespace rtcp
} // namespace xrtc
