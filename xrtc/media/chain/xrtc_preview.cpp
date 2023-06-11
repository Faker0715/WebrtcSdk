//
// Created by faker on 2023/6/10.
//

#include "xrtc/media/chain/xrtc_preview.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "xrtc/base/xrtc_global.h"
namespace xrtc{

    XRTCPreview::XRTCPreview(IVideoSource *video_source, XRTCRender *render):
            current_thread_(rtc::Thread::Current()),
            video_source_(video_source),
            render_(render),
            xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
            d3d9_render_sink_(std::make_unique<D3D9RenderSink>()){

    }
    XRTCPreview::~XRTCPreview() {

    }

    void XRTCPreview::Start() {
        RTC_LOG(LS_INFO) << "XRTCPreview Start call";
        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            RTC_LOG(LS_INFO) << "XRTCPreview Start PostTask";
            XRTCError err = XRTCError::kNoErr;
            do {
                if (has_start_) {
                    RTC_LOG(LS_WARNING) << "XRTCPreview Start has start, ignore";
                    break;
                }
                // 设置视频源
                if(!video_source_){
                    err = XRTCError::kPreviewNoVideoSource;
                    RTC_LOG(LS_WARNING) << "XRTCPreview failed: no video source";
                    break;
                }
                video_source_->AddConsumer(xrtc_video_source_.get());
                AddMediaObject(xrtc_video_source_.get());
                AddMediaObject(d3d9_render_sink_.get());

            } while (false);
        }));
    }

    void XRTCPreview::Stop() {
    }



}