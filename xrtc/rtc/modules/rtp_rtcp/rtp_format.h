//
// Created by faker on 2023/6/19.
//

#ifndef XRTCSDK_RTP_FORMAT_H
#define XRTCSDK_RTP_FORMAT_H


#include <memory>
#include <vector>

#include <api/video/video_codec_type.h>
#include <api/array_view.h>

namespace xrtc {

    class RtpPacketToSend;

    class RtpPacketizer {
    public:
        struct PayloadLimits {
            int max_payload_len = 1200;
            // nalu 拆分的包不一样，第一个包 最后一个包和中间的包的长度不一样
            int single_packet_reduction_len = 0;
            int first_packet_reduction_len = 0;
            int last_packet_reduction_len = 0;
        };

        struct Config {
            PayloadLimits limits;
        };

        static std::unique_ptr<RtpPacketizer> Create(webrtc::VideoCodecType type,
                                                     rtc::ArrayView<const uint8_t> payload,
                                                     const RtpPacketizer::Config& config);

        std::vector<int> SplitAboutEqual(size_t payload_size, const PayloadLimits& limits);

        virtual ~RtpPacketizer() = default;
        virtual size_t NumPackets() = 0;
        virtual bool NextPacket(RtpPacketToSend* packet) = 0;
    };

}



#endif //XRTCSDK_RTP_FORMAT_H
