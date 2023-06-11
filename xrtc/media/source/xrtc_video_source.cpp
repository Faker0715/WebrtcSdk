//
// Created by faker on 2023/6/11.
//

#include "xrtc_video_source.h"
#include "xrtc/media/base/out_pin.h"
namespace xrtc{
    void XRTCVideoSource::OnFrame(std::shared_ptr<MediaFrame> frame) {


    }

    XRTCVideoSource::XRTCVideoSource() : out_pin_(std::make_unique<OutPin>(this)){
        MediaFormat fmt;
        fmt.media_type = MainMediaType::kMainTypeVideo;
        fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
        out_pin_->set_format(fmt);
    }

    XRTCVideoSource::~XRTCVideoSource() {

    }

    bool XRTCVideoSource::Start() {
        return true;
    }

    void XRTCVideoSource::Stop() {

    }
}