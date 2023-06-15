//
// Created by faker on 2023/6/15.
//

#include "xrtc_pusher.h"

namespace xrtc{

    XRTCPusher::XRTCPusher(IVideoSource *video_source) : video_source_(video_source),
                             current_thread_(rtc::Thread::Current()) {

    }

    XRTCPusher::~XRTCPusher() {

    }

    void XRTCPusher::StartPush(const std::string &url) {

    }
}