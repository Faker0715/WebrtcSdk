//
// Created by faker on 2023/6/10.
//

#ifndef XRTCSDK_XRTC_PREVIEW_H
#define XRTCSDK_XRTC_PREVIEW_H
#include "xrtc/xrtc.h"
#include "xrtc/device/xrtc_render.h"
#include "xrtc/media/base/media_chain.h"

// 实现摄像头预览
namespace xrtc{
    class MediaChain;
    // 需要导出给上层来使用
    class XRTC_API XRTCPreview : public MediaChain{
    public:
        ~XRTCPreview();
        void Start() override;
        void Stop() override;


    private:
        // 不允许直接创建
        XRTCPreview(IVideoSource* video_source, XRTCRender* render);

        friend class XRTCEngine;

    private:
        IVideoSource* video_source_;
        XRTCRender* render_;
    };
}


#endif //XRTCSDK_XRTC_PREVIEW_H
