//
// Created by faker on 2023/6/16.
//

#include "x264_encoder_filter.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/media/base/out_pin.h"
namespace xrtc{


    X264EncoderFilter::X264EncoderFilter() : in_pin_(std::make_unique<InPin>(this)),
                                             out_pin_(std::make_unique<OutPin>(this)){
        MediaFormat fmt_in;
        fmt_in.media_type = MainMediaType::kMainTypeVideo;
        fmt_in.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
        in_pin_->set_format(fmt_in);


    }

    X264EncoderFilter::~X264EncoderFilter() {

    }

    bool X264EncoderFilter::Start() {
        return false;
    }

    void X264EncoderFilter::Stop() {

    }

    void X264EncoderFilter::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        return ;
    }
}