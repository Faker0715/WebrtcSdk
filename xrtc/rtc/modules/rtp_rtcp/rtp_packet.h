//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_PACKET_H
#define XRTCSDK_RTP_PACKET_H


#include <vector>

#include <rtc_base/copy_on_write_buffer.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_header_extension_map.h"

namespace xrtc {

    class RtpPacket {
    public:
        RtpPacket();
        RtpPacket(const RtpHeaderExtensionMap* extensions);
        RtpPacket(const RtpHeaderExtensionMap* extensions, size_t capacity);

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

        template <typename Extension>
        bool ReserveExtension();
        rtc::ArrayView<uint8_t> AllocateExtension(RTPExtensionType type, size_t length);

    private:
        struct ExtensionInfo {
            ExtensionInfo(uint8_t id) : ExtensionInfo(id, 0, 0) {}
            ExtensionInfo(uint8_t id, size_t length, size_t offset) :
                    id(id), length(length), offset(offset) {}

            uint8_t id;    // 扩展的id
            size_t length; // 扩展的长度
            size_t offset; // 扩展在RTP头中的偏移量，不包含扩展的头部字节
        };

        rtc::ArrayView<uint8_t> AllocateRawExtension(uint8_t id,
                                                     size_t length);
        const ExtensionInfo* FindExtension(uint8_t id);
        void PromoteToTwoByteHeaderExtension();
        uint16_t SetExtensionLengthMaybeAddZeroPadding(size_t extension_offset);

    private:
        bool marker_;
        uint8_t payload_type_;
        uint16_t sequence_number_;
        uint32_t timestamp_;
        uint32_t ssrc_;
        size_t payload_offset_;
        size_t payload_size_;
        size_t padding_size_;

        RtpHeaderExtensionMap extensions_;
        std::vector<ExtensionInfo> extension_entries_;
        // 添加的扩展的总长度
        size_t extension_size_ = 0;

        rtc::CopyOnWriteBuffer buffer_;
    };

    template <typename Extension>
    bool RtpPacket::ReserveExtension() {
        auto buffer = AllocateExtension(Extension::kId, Extension::kValueSizeBytes);
        if (buffer.empty()) {
            return false;
        }

        memset(buffer.data(), 0, Extension::kValueSizeBytes);
        return true;
    }

} // namespace xrtc


#endif //XRTCSDK_RTP_PACKET_H
