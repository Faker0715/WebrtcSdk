//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_PACKET_H
#define XRTCSDK_RTP_PACKET_H


#include <rtc_base/copy_on_write_buffer.h>

namespace xrtc {

    class RtpPacket {
    public:
        RtpPacket();
        RtpPacket(size_t capacity);

        uint16_t sequence_number() const { return sequence_number_; }
        uint32_t ssrc() const { return ssrc_; }
        bool marker() const { return marker_; }
        uint32_t timestamp() const { return timestamp_; }
        rtc::ArrayView<const uint8_t> payload() const {
            return rtc::MakeArrayView(data() + payload_offset_, payload_size_);
        }

        size_t header_size() const { return payload_offset_; }
        size_t payload_size() const { return payload_size_; }
        size_t padding_size() const { return padding_size_; }

        const uint8_t* data() const { return buffer_.cdata(); }
        size_t size() {
            return payload_offset_ + payload_size_ + padding_size_;
        }
        size_t capacity() { return buffer_.capacity(); }
        size_t FreeCapacity() { return capacity() - size(); }
        void Clear();

        void SetMarker(bool marker_bit);
        void SetPayloadType(uint8_t payload_type);
        void SetSequenceNumber(uint16_t seq_no);
        void SetTimestamp(uint32_t ts);
        void SetSsrc(uint32_t ssrc);
        uint8_t* SetPayloadSize(size_t bytes_size);

        uint8_t* AllocatePayload(size_t payload_size);

        uint8_t* WriteAt(size_t offset) {
            return buffer_.MutableData() + offset;
        }

        void WriteAt(size_t offset, uint8_t byte) {
            buffer_.MutableData()[offset] = byte;
        }

    private:
        bool marker_;
        uint8_t payload_type_;
        uint16_t sequence_number_;
        uint32_t timestamp_;
        uint32_t ssrc_;
        size_t payload_offset_;
        size_t payload_size_;
        size_t padding_size_;
        rtc::CopyOnWriteBuffer buffer_;
    };

} // namespace xrtc




#endif //XRTCSDK_RTP_PACKET_H
