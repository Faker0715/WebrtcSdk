//
// Created by faker on 2023/11/8.
//

#ifndef XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_
#define XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_

#include <vector>

#include <api/units/time_delta.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/rtpfb.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"

namespace xrtc {
    namespace rtcp {

        class TransportFeedback : public Rtpfb {
        public:
            class ReceivePacket {
            public:
                ReceivePacket(uint16_t sequence_number, int16_t delta_ticks) :
                        sequence_number_(sequence_number),
                        delta_ticks_(delta_ticks),
                        received_(true) { }
                ReceivePacket(uint16_t sequence_number) :
                        sequence_number_(sequence_number),
                        received_(false) {}

                uint16_t sequence_number() const {
                    return sequence_number_;
                }

                int16_t delta_ticks() const { return delta_ticks_; }
                int32_t delta_us() const { return delta_ticks_ * kDeltaScaleFactor; }
                webrtc::TimeDelta delta() const {
                    return webrtc::TimeDelta::Micros(delta_us());
                }

                bool received() const { return received_; }

                std::string ToString() const;

            private:
                uint16_t sequence_number_;
                int16_t delta_ticks_ = 0;
                bool received_;
            };

            static const uint8_t kFeedbackMessageType = 15;
            static const int kDeltaScaleFactor = 250; // 250us

            const std::vector<ReceivePacket>& AllPackets() const {
                return all_packets_;
            }

            const std::vector<ReceivePacket>& ReceivedPackets() const {
                return received_packets_;
            }

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
                void Clear();

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
                bool has_large_delta_ = false;
            };

            void Clear();

        private:
            uint16_t base_seq_no_ = 0;
            int32_t base_time_ticks_ = 0;
            uint8_t feedback_seq_ = 0;
            LastChunk last_chunk_;
            // 存放所有的数据包，包含没有收到的数据包
            std::vector<ReceivePacket> all_packets_;
            // 存放收到的数据包
            std::vector<ReceivePacket> received_packets_;
            bool include_lost_ = true;
            bool include_timestamps_ = true;
            size_t size_bytes_ = 0;
        };

    } // namespace rtcp
} // namespace xrtc

#endif // XRTCSDK_XRTC_MODULES_RTP_RTCP_RTCP_PACKET_TRANSPORT_FEEDBACK_H_