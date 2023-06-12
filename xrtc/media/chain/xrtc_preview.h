//
// Created by faker on 2023/6/10.
//

#ifndef XRTCSDK_XRTC_PREVIEW_H
#define XRTCSDK_XRTC_PREVIEW_H
#include <rtc_base/thread.h>

#include "xrtc/xrtc.h"
#include "xrtc/device/xrtc_render.h"
#include "xrtc/media/base/media_chain.h"
#include "xrtc/media/source/xrtc_video_source.h"
#include "xrtc/media/sink/d3d9_render_sink.h"


// 实现摄像头预览
namespace xrtc{
    // 需要导出给上层来使用
    class XRTC_API XRTCPreview : public MediaChain{
    public:
        ~XRTCPreview();
        void Start() override;
        void Stop() override;
        void Destroy() override;;


    private:
        // 不允许直接创建
        XRTCPreview(IVideoSource* video_source, XRTCRender* render);

        friend class XRTCEngine;

    private:
        rtc::Thread* current_thread_;
        IVideoSource* video_source_;
        XRTCRender* render_;
        std::unique_ptr<XRTCVideoSource> xrtc_video_source_;
        std::unique_ptr<D3D9RenderSink> d3d9_render_sink_;
        bool has_start_ = false;
    };
}


#endif //XRTCSDK_XRTC_PREVIEW_H
