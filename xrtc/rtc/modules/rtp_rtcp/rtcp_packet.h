//
// Created by faker on 2023/6/25.
//

#ifndef XRTCSDK_RTCP_PACKET_H
#define XRTCSDK_RTCP_PACKET_H


#include <stdint.h>

#include <api/function_view.h>
#include <api/array_view.h>

namespace xrtc {
    namespace rtcp {

        class RtcpPacket {
        public:
            using PacketReadyCallback = rtc::FunctionView<
                    void(rtc::ArrayView<const uint8_t> packet)>;

            virtual ~RtcpPacket() {}

            void SetSenderSsrc(uint32_t ssrc) {
                sender_ssrc_ = ssrc;
            }

            uint32_t sender_ssrc() const { return sender_ssrc_; }

            virtual size_t BlockLength() const = 0;

            virtual bool Create(uint8_t* packet,
                                size_t* index,
                                size_t max_length,
                                PacketReadyCallback callback) const = 0;

        protected:
            static const size_t kHeaderSize = 4;
            RtcpPacket() = default;

            static void CreateHeader(size_t count_or_fmt,
                                     uint8_t packet_type,
                                     uint16_t block_length,
                                     uint8_t* buffer,
                                     size_t* pos);

            static void CreateHeader(size_t count_or_fmt,
                                     uint8_t packet_type,
                                     uint16_t block_length,
                                     bool padding,
                                     uint8_t* buffer,
                                     size_t* pos);

            bool OnBufferFull(uint8_t* packet,
                              size_t* index,
                              PacketReadyCallback callback) const;

            size_t HeaderLength() const;

        private:
            uint32_t sender_ssrc_;
        };

    } // namespace rtcp

} // namespace xrtc



#endif //XRTCSDK_RTCP_PACKET_H
