//
// Created by faker on 2023/6/15.
//

#ifndef XRTCSDK_XRTC_PUSHER_H
#define XRTCSDK_XRTC_PUSHER_H

#include "xrtc/xrtc.h"
#include "rtc_base/thread.h"
#include "xrtc/media/base/media_chain.h"

namespace xrtc{
    class XRTC_API XRTCPusher{
    public:
        void StartPush(const std::string& url);

    private:
        XRTCPusher(IVideoSource* video_source);
        ~XRTCPusher();


        friend class XRTCEngine;
    private:
        IVideoSource* video_source_;
        std::string url_;
        rtc::Thread* current_thread_;
        std::unique_ptr<MediaChain> media_chain_;


    };

}


#endif //XRTCSDK_XRTC_PUSHER_H
