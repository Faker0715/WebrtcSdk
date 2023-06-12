//
// Created by faker on 2023/6/10.
//

#include "xrtc/media/chain/xrtc_preview.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "xrtc/base/xrtc_global.h"
#include "xrtc/base/xrtc_json.h"

namespace xrtc {

    XRTCPreview::XRTCPreview(IVideoSource *video_source, XRTCRender *render) :
            current_thread_(rtc::Thread::Current()),
            video_source_(video_source),
            render_(render),
            xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
            d3d9_render_sink_(std::make_unique<D3D9RenderSink>()) {

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
                if (!video_source_) {
                    err = XRTCError::kPreviewNoVideoSourceErr;
                    RTC_LOG(LS_WARNING) << "XRTCPreview failed: no video source";
                    break;
                }
                video_source_->AddConsumer(xrtc_video_source_.get());
                AddMediaObject(xrtc_video_source_.get());
                AddMediaObject(d3d9_render_sink_.get());
                if (!ConnectMediaObject(xrtc_video_source_.get(), d3d9_render_sink_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "XRTCPreview failed: connect failed";
                    break;
                }
                RTC_LOG(LS_INFO) << "XRTCPreview Start success";

                //SetupChain();
                JsonObject json_config;
                JsonObject j_d3d9_render_sink;
                j_d3d9_render_sink["hwnd"] = (long long)render_->canvas();
                json_config["d3d9_render_sink"] = j_d3d9_render_sink;
                SetupChain(JsonValue(json_config).ToJson());




                if(!StartChain()){
                    err = XRTCError::kChainStartErr;
                    RTC_LOG(LS_WARNING) << "XRTCPreview failed: start chain failed";
                    break;
                }
                has_start_ = true;
            } while (false);
            if(XRTCGlobal::Instance()->engine_observer()){
                if(err == XRTCError::kNoErr){
                    XRTCGlobal::Instance()->engine_observer()->OnPreviewSuccess(this);
                }else{
                    XRTCGlobal::Instance()->engine_observer()->OnPreviewFailed(this, err);
                }
            }
        }));
    }

    void XRTCPreview::Stop() {
        RTC_LOG(LS_INFO) << "XRTCPreview Stop call";
        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            RTC_LOG(LS_INFO) << "XRTCPreview Stop PostTask";
        }));
    }

    void XRTCPreview::Destroy() {
        RTC_LOG(LS_INFO) << "XRTCPreview Destroy call";
        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            RTC_LOG(LS_INFO) << "XRTCPreview Destroy PostTask";
            delete this;
        }));
    }


}