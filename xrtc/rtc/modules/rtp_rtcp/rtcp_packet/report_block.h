//
// Created by faker on 2023/6/25.
//

#ifndef XRTCSDK_REPORT_BLOCK_H
#define XRTCSDK_REPORT_BLOCK_H


#include <stdint.h>

namespace xrtc {
    namespace rtcp {

        class ReportBlock {
        public:
            static const size_t kLength = 24;
            ReportBlock() = default;
            ~ReportBlock() = default;

            bool Parse(const uint8_t* buffer, size_t len);

            uint32_t source_ssrc() const { return source_ssrc_; }
            uint32_t last_sr() const { return last_sr_; }
            uint32_t delay_since_last_sr() const { return delay_since_last_sr_; }
            int32_t packets_lost() const { return cumulative_packets_lost_; }
            uint8_t fraction_lost() const { return fraction_lost_; }
            uint32_t jitter() const { return jitter_; }

        private:
            uint32_t source_ssrc_ = 0;
            uint8_t fraction_lost_ = 0;
            int32_t cumulative_packets_lost_ = 0;
            uint32_t exetened_highest_sequence_number_ = 0;
            uint32_t jitter_ = 0;
            uint32_t last_sr_ = 0;
            uint32_t delay_since_last_sr_ = 0;
        };

    } // namespace rtcp
} // namespace xrtc



#endif //XRTCSDK_REPORT_BLOCK_H
