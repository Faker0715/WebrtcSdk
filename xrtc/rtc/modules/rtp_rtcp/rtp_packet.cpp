#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"

#include <rtc_base/logging.h>
#include <api/rtp_parameters.h>
#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {

    const size_t kDefaultCapacity = 1500;
    const size_t kFixedHeaderSize = 12;
    const uint8_t kRtpVersion = 2;
    const uint16_t kOneByteHeaderExtensionProfileId = 0xBEDE;
    const uint16_t kTwoByteHeaderExtensionProfileId = 0x1000;
    const size_t kOneByteHeaderExtensionLength = 1;
    const size_t kTwoByteHeaderExtensionLength = 2;

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
    RtpPacket::RtpPacket() : RtpPacket(nullptr, kDefaultCapacity) {
    }

    RtpPacket::RtpPacket(const RtpHeaderExtensionMap* extensions) :
            RtpPacket(extensions, kDefaultCapacity)
    {
    }

    RtpPacket::RtpPacket(const RtpHeaderExtensionMap* extensions, size_t capacity) :
            extensions_(extensions ? *extensions : RtpHeaderExtensionMap()),
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

    rtc::ArrayView<uint8_t> RtpPacket::AllocateExtension(RTPExtensionType type,
                                                         size_t length)
    {
        // 长度校验
        if (length == 0 || length > webrtc::RtpExtension::kMaxValueSize) {
            return nullptr;
        }

        // 检查扩展是否已经注册
        uint8_t id = extensions_.GetId(type);
        if (id == RtpHeaderExtensionMap::kInvalidId) { // 扩展没有注册
            return nullptr;
        }

        return AllocateRawExtension(id, length);
    }

    rtc::ArrayView<uint8_t> RtpPacket::AllocateRawExtension(uint8_t id,
                                                            size_t length)
    {
        // 判断将要添加的扩展，是否已经添加过了
        const ExtensionInfo* extension_entry = FindExtension(id);
        if (extension_entry) {
            // 扩展已经添加过
            if (extension_entry->length == length) {
                return rtc::MakeArrayView<uint8_t>(
                        WriteAt(extension_entry->offset), length);
            }

            RTC_LOG(LS_WARNING) << "length mismatch id: " << id
                                << ", expected length: " << static_cast<size_t>(extension_entry->length)
                                << ", received length: " << length;

            return nullptr;
        }

        // 如果RTP的负载已经设置，不允许在添加新的头部扩展
        if (payload_size_ > 0) {
            RTC_LOG(LS_WARNING) << "Cannot add new extension id: " << id
                                << " after payload already set";
            return nullptr;
        }

        // 如果RTP的padding已经设置，不允许在添加新的头部扩展
        if (padding_size_ > 0) {
            RTC_LOG(LS_WARNING) << "Cannot add new extension id: " << id
                                << " after padding already set";
            return nullptr;
        }

        // 首先，获得扩展在RTP包头中的偏移量
        size_t num_csrcs = data()[0] & 0x0F;
        size_t extensions_offset = kFixedHeaderSize + num_csrcs * 4 + 4;

        // 判断将要添加的扩展，是使用一字节头还是两字节头
        bool two_bytes_header_required = id > webrtc::RtpExtension::kOneByteHeaderExtensionMaxId
                                         || length > webrtc::RtpExtension::kOneByteHeaderExtensionMaxValueSize
                                         || length == 0;

        uint16_t profile_id;
        // 之前已经添加过扩展
        if (extension_size_ > 0) {
            profile_id = webrtc::ByteReader<uint16_t>::ReadBigEndian(
                    WriteAt(extensions_offset - 4));
            // 判断是否要将1字节头提升为2字节头
            if (profile_id == kOneByteHeaderExtensionProfileId &&
                two_bytes_header_required)
            {
                // 原始的已添加的扩展都是1字节头，但是新添加的扩展需要两字节头
                // 因此，需要将扩展头提升为2字节头
                // 在提升之前，需要判断容量是否足够
                // 提升之后的扩展头长度 = 原来的扩展头长度
                // + 已经存在的扩展头个数 * 1字节 + 当前新扩展的头部
                // + 当前新扩展的数据长度
                size_t expected_extension_size = extension_size_ +
                                                 extension_entries_.size() + kTwoByteHeaderExtensionLength
                                                 + length;
                if (extensions_offset + expected_extension_size > capacity()) {
                    RTC_LOG(LS_WARNING) << "No enough buffer space change to "
                                        << " two-byte header extension and add new extension, id: "
                                        << id;
                    return nullptr;
                }

                PromoteToTwoByteHeaderExtension();
                profile_id = kTwoByteHeaderExtensionProfileId;
            }
        }
        else { // 第一次添加头部扩展
            profile_id = two_bytes_header_required
                         ? kTwoByteHeaderExtensionLength
                         : kOneByteHeaderExtensionLength;
        }

        // 添加新的扩展
        // 计算添加新扩展之后，扩展的新的总长度
        size_t extension_header_size =
                (profile_id == kOneByteHeaderExtensionProfileId)
                ? kOneByteHeaderExtensionLength
                : kTwoByteHeaderExtensionLength;
        size_t new_extension_size = extension_size_ + extension_header_size
                                    + length;
        if (new_extension_size > capacity()) {
            RTC_LOG(LS_WARNING) << "No enought buffer space to add new extension, id: "
                                << id;
            return nullptr;
        }

        // 如果是第一个扩展的话，还需要写入profile_id
        if (0 == extension_size_) {
            // 设置X标记位置
            WriteAt(0, data()[0] | 0x10);
            // 写入profile_id
            webrtc::ByteWriter<uint16_t>::WriteBigEndian(
                    WriteAt(extensions_offset - 4),
                    profile_id);
        }

        // 写入扩展的头部
        if (profile_id == kOneByteHeaderExtensionProfileId) {
            uint8_t one_byte_header = (id << 4);
            one_byte_header |= static_cast<uint8_t>(length - 1);
            WriteAt(extensions_offset + extension_size_, one_byte_header);
        }
        else {
            WriteAt(extensions_offset + extension_size_, id);
            WriteAt(extensions_offset + extension_size_ + 1,
                    static_cast<uint8_t>(length));
        }

        // 将新添加的扩展，保存到extension_entries_
        size_t extension_info_offset = extensions_offset +
                                       extension_size_ + extension_header_size;
        extension_entries_.emplace_back(id, static_cast<uint8_t>(length),
                                        extension_info_offset);

        extension_size_ = new_extension_size;

        // 更新扩展的总长度
        uint16_t extension_size_padded =
                SetExtensionLengthMaybeAddZeroPadding(extensions_offset);
        payload_offset_ = extensions_offset + extension_size_padded;
        buffer_.SetSize(payload_offset_);

        // 返回新添加的扩展的地址
        return rtc::MakeArrayView(WriteAt(extension_info_offset),
                                  length);
    }

    const RtpPacket::ExtensionInfo* RtpPacket::FindExtension(uint8_t id) {
        for (const ExtensionInfo& entry : extension_entries_) {
            if (id == entry.id) {
                return &entry;
            }
        }
        return nullptr;
    }

    void RtpPacket::PromoteToTwoByteHeaderExtension() {
        // 首先，获得当前扩展的偏移量
        size_t num_csrcs = data()[0] & 0x0F; // 贡献源的个数
        // 偏移量不包含自身的头部
        size_t extension_offset = kFixedHeaderSize + num_csrcs * 4 + 4;

        size_t write_data_delta = extension_entries_.size();
        for (auto extension_entry = extension_entries_.rbegin();
             extension_entry != extension_entries_.rend();
             ++extension_entry)
        {
            size_t read_index = extension_entry->offset;
            // 移动之后的扩展的偏移量
            size_t write_index = read_index + write_data_delta;
            // 修改当前的扩展的偏移量
            extension_entry->offset = write_index;
            // 将原始的扩展数据，移动到新的位置
            memmove(WriteAt(write_index), data() + read_index, extension_entry->length);
            // 重新写入新的扩展头部数据
            // 向两字节头部写入Length信息
            WriteAt(--write_index, extension_entry->length);
            // 向两字节头部写入ID信息
            WriteAt(--write_index, extension_entry->id);

            --write_data_delta;
        }

        // 更新profile_id
        webrtc::ByteWriter<uint16_t>::WriteBigEndian(
                WriteAt(extension_offset - 4), kTwoByteHeaderExtensionProfileId);
        // 更新扩展的总长度
        extension_size_ += extension_entries_.size();
        uint16_t extension_size_padded =
                SetExtensionLengthMaybeAddZeroPadding(extension_offset);
        // 修改负载的偏移量
        payload_offset_ = extension_offset + extension_size_padded;
        buffer_.SetSize(payload_offset_);
    }

    uint16_t RtpPacket::SetExtensionLengthMaybeAddZeroPadding(size_t extension_offset) {
        // 确保4字节对齐
        uint16_t extension_words = (extension_size_ + 3) / 4;
        webrtc::ByteWriter<uint16_t>::WriteBigEndian(
                WriteAt(extension_offset - 2), extension_words);
        // 需要填充的字节数
        size_t extension_padding_size = 4 * extension_words - extension_size_;
        // 在扩展的尾部填充0
        memset(WriteAt(extension_offset + extension_size_), 0,
               extension_padding_size);
        return 4 * extension_words;
    }

} // namespace xrtc