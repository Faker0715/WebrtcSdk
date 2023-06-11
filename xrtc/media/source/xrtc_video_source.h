//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_XRTC_VIDEO_SOURCE_H
#define XRTCSDK_XRTC_VIDEO_SOURCE_H

#include "xrtc/xrtc.h"
#include "xrtc/media/base/media_chain.h"

namespace xrtc {
    class XRTCVideoSource : public IXRTCConsumer, public MediaObject {
    public:
        XRTCVideoSource();

        ~XRTCVideoSource() override;

        bool Start() override;

        void Stop() override;

        void OnFrame(std::shared_ptr<MediaFrame> frame) override;


    };
}


#endif //XRTCSDK_XRTC_VIDEO_SOURCE_H
