//
// Created by faker on 2023/6/27.
//

#ifndef XRTCSDK_COMMON_HEADER_H
#define XRTCSDK_COMMON_HEADER_H


#include <stdint.h>

namespace xrtc {
    namespace rtcp {

        class CommonHeader {
        public:
            static const size_t kHeaderSizeBytes = 4;

            CommonHeader();
            ~CommonHeader();

            bool Parse(const uint8_t* buffer, size_t len);
            // 因为是复合包，返回下一个RTCP包的地址
            const uint8_t* NextPacket() const {
                return payload_ + padding_size_ + payload_size_;
            }

            uint8_t packet_type() const { return packet_type_; }
            uint8_t count() const { return count_or_fmt_; }
            uint8_t fmt() const { return count_or_fmt_; }
            uint32_t payload_size() const { return payload_size_; }
            const uint8_t* payload() const { return payload_; }

        private:
            uint8_t count_or_fmt_ = 0;
            uint8_t packet_type_ = 0;
            uint32_t padding_size_ = 0;
            uint32_t payload_size_ = 0;
            const uint8_t* payload_ = nullptr;
        };

    } // namespace rtcp
} // namespace xrtc



#endif //XRTCSDK_COMMON_HEADER_H
