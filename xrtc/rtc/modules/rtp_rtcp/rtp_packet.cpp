//
// Created by faker on 2023/6/23.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"

#include <rtc_base/logging.h>
#include "modules/rtp_rtcp/source/byte_io.h"

namespace xrtc {

    const size_t kDefaultCapacity = 1500;
    const size_t kFixedHeaderSize = 12;
    const uint8_t kRtpVersion = 2;

//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|X|  CC   |M|     PT      |       sequence number         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           timestamp                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           synchronization source (SSRC) identifier            |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |            Contributing source (CSRC) identifiers             |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |  header eXtension profile id  |       length in 32bits        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Extensions                           |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                           Payload                             |
// |             ....              :  padding...                   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               padding         | Padding size  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    RtpPacket::RtpPacket() : RtpPacket(kDefaultCapacity) {
    }

    RtpPacket::RtpPacket(size_t capacity) :
            buffer_(capacity)
    {
        Clear();
    }

    void RtpPacket::Clear() {
        marker_ = false;
        payload_type_ = 0;
        sequence_number_ = 0;
        timestamp_ = 0;
        ssrc_ = 0;
        payload_offset_ = kFixedHeaderSize;
        payload_size_ = 0;
        padding_size_ = 0;

        buffer_.SetSize(kFixedHeaderSize);
        // 写入RTP版本信息
        WriteAt(0, kRtpVersion << 6);
    }

    void RtpPacket::SetMarker(bool marker_bit) {
        marker_ = marker_bit;
        if (marker_bit) {
            WriteAt(1, data()[1] | 0x80);
        }
        else {
            WriteAt(1, data()[1] & 0x7F);
        }
    }

    void RtpPacket::SetPayloadType(uint8_t payload_type) {
        payload_type_ = payload_type;
        WriteAt(1, (data()[1] & 0x80) | payload_type);
    }

    void RtpPacket::SetSequenceNumber(uint16_t seq_no) {
        sequence_number_ = seq_no;
        webrtc::ByteWriter<uint16_t>::WriteBigEndian(WriteAt(2), seq_no);
    }

    void RtpPacket::SetTimestamp(uint32_t ts) {
        timestamp_ = ts;
        webrtc::ByteWriter<uint32_t>::WriteBigEndian(WriteAt(4), ts);
    }

    void RtpPacket::SetSsrc(uint32_t ssrc) {
        ssrc_ = ssrc;
        webrtc::ByteWriter<uint32_t>::WriteBigEndian(WriteAt(8), ssrc);
    }

    uint8_t* RtpPacket::SetPayloadSize(size_t bytes_size) {
        if (payload_offset_ + bytes_size > capacity()) {
            RTC_LOG(LS_WARNING) << "set payload size failed, no enough space in buffer";
            return nullptr;
        }

        payload_size_ = bytes_size;
        buffer_.SetSize(payload_offset_ + payload_size_);
        return WriteAt(payload_offset_);
    }

    uint8_t* RtpPacket::AllocatePayload(size_t payload_size) {
        SetPayloadSize(0);
        return SetPayloadSize(payload_size);
    }

} // namespace xrtc
