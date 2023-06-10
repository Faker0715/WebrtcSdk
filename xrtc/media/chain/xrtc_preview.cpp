//
// Created by faker on 2023/6/10.
//

#include "xrtc_preview.h"

namespace xrtc{
    XRTCPreview::XRTCPreview(IVideoSource *video_source, XRTCRender *render) : video_source_(video_source),
                                                                              render_(render) {

    }

    XRTCPreview::~XRTCPreview() {

    }

    void XRTCPreview::Start() {
    }

    void XRTCPreview::Stop() {
    }
}