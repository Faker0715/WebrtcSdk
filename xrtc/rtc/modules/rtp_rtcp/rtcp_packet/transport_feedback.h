//
// Created by faker on 2023/11/8.
//

#ifndef XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_
#define XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_

#include <vector>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"

namespace xrtc {
    namespace rtcp {

        class TransportFeedback : public Rtpfb {
        public:
            static const uint8_t kFeedbackMessageType = 15;

            bool Parse(const rtcp::CommonHeader& packet);

            size_t BlockLength() const override;

            virtual bool Create(uint8_t* packet,
                                size_t* index,
                                size_t max_length,
                                PacketReadyCallback callback) const override;

        private:
            class LastChunk {
            public:
                void Decode(uint16_t chunk, size_t max_size);
                void AppendTo(std::vector<uint8_t>* deltas);

            private:
                static const size_t kRunLengthCapacity = 0x1fff;
                static const size_t kOneBitCapacity = 14;
                static const size_t kTwoBitCapacity = 7;
                static const size_t kVectorCapacity = kOneBitCapacity;
                static const uint8_t kLarge = 2;

                void DecodeRunLength(uint16_t chunk, size_t max_size);
                void DecodeOneBit(uint16_t chunk, size_t max_size);
                void DecodeTwoBit(uint16_t chunk, size_t max_size);

            private:
                uint8_t delta_sizes_[kVectorCapacity];
                size_t size_ = 0;
                bool all_same_ = false;
                bool has_large_data_ = false;
            };

        private:
            uint16_t base_seq_no_ = 0;
            int32_t base_time_ticks_ = 0;
            uint8_t feedback_seq_ = 0;
            LastChunk last_chunk_;
        };

    } // namespace rtcp
} // namespace xrtc

#endif // XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_