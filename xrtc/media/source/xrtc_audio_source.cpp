//
// Created by faker on 2023/6/29.
//

#include "xrtc/media/source/xrtc_audio_source.h"

#include "xrtc/media/base/out_pin.h"
#include "xrtc/media/base/media_frame.h"

namespace xrtc {

    XRTCAudioSource::XRTCAudioSource() :
            out_pin_(std::make_unique<OutPin>(this))
    {
        MediaFormat fmt;
        fmt.media_type = MainMediaType::kMainTypeAudio;
        fmt.sub_fmt.audio_fmt.type = SubMediaType::kSubTypePcm;
        out_pin_->set_format(fmt);
    }

    XRTCAudioSource::~XRTCAudioSource() {
    }

    bool XRTCAudioSource::Start() {
        return true;
    }

    void XRTCAudioSource::Stop() {
    }

    void XRTCAudioSource::OnFrame(std::shared_ptr<MediaFrame> frame) {
        if (out_pin_) {
            out_pin_->PushMediaFrame(frame);
        }
    }

} // namespace xrtc
