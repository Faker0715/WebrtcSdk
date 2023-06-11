//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_XRTC_VIDEO_SOURCE_H
#define XRTCSDK_XRTC_VIDEO_SOURCE_H

#include "xrtc/xrtc.h"

namespace xrtc{
    class XRTCVideoSource : public IXRTCConsumer{
    public:
        XRTCVideoSource() ;
        ~XRTCVideoSource() override ;
        void OnFrame(std::shared_ptr<MediaFrame> frame) override;


    };
}


#endif //XRTCSDK_XRTC_VIDEO_SOURCE_H
