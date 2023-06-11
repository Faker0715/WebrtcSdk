//
// Created by faker on 2023/6/11.
//

#include "d3d9_render_sink.h"
#include "xrtc/media/base/in_pin.h"
namespace xrtc{

    D3D9RenderSink::D3D9RenderSink():
        in_pin_(std::make_unique<InPin>(this)){
        MediaFormat fmt;
        fmt.media_type = MainMediaType::kMainTypeVideo;
        fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
        in_pin_->set_format(fmt);
    }

    D3D9RenderSink::~D3D9RenderSink() {

    }

    bool D3D9RenderSink::Start() {
        return false;
    }

    void D3D9RenderSink::Stop() {

    }
}