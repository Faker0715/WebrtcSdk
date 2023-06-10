//
// Created by faker on 2023/6/10.
//

#include "xrtc/media/chain/xrtc_preview.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "xrtc/base/xrtc_global.h"
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