//
// Created by faker on 2023/6/27.
//

#ifndef XRTCSDK_NACK_H
#define XRTCSDK_NACK_H


#include <vector>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"

namespace xrtc {
    namespace rtcp {

        class CommonHeader;

        class Nack : public Rtpfb {
        public:
            static const uint8_t kFeedbackMessageType = 1;
            Nack() = default;
            ~Nack() override = default;

            size_t BlockLength() const override;

            bool Create(uint8_t* packet,
                        size_t* index,
                        size_t max_length,
                        PacketReadyCallback callback) const override;

            bool Parse(const rtcp::CommonHeader& packet);

            const std::vector<uint16_t> packet_ids() const {
                return packet_ids_;
            }

        private:
            static const size_t kNackItemLength = 4;
            struct PackedNack {
                uint16_t first_pid;
                uint16_t bitmask;
            };

            void Unpack();

            std::vector<PackedNack> packed_;
            std::vector<uint16_t> packet_ids_;
        };

    } // namespace rtcp
} // namespace xrtc

#endif //XRTCSDK_NACK_H
