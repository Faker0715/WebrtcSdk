//
// Created by faker on 2023/6/16.
//

#include "xrtc_push_stream.h"
#include "rtc_base/logging.h"
#include "xrtc/base/xrtc_json.h"
#include "xrtc/media/base/xrtc_pusher.h"
#include "xrtc/base/xrtc_global.h"

namespace xrtc{

    XRTCPushStream::XRTCPushStream(XRTCPusher *pusher, IVideoSource *video_source):
            pusher_(pusher),
            video_source_(video_source),
            xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
            x264_encoder_filter_(std::make_unique<X264EncoderFilter>()),
            xrtc_media_sink_(std::make_unique<XRTCMediaSink>(this))
    {

    }

    XRTCPushStream::~XRTCPushStream() {

    }

    void XRTCPushStream::Start() {

        RTC_LOG(LS_INFO) << "XRTCPushStream Start";
        XRTCError err = XRTCError::kNoErr;
        do{
            if(!video_source_){
                err = XRTCError::kPushNoVideoSourceErr;
                RTC_LOG(LS_WARNING) << "XRTCPushStream Start failed, no video source";
                break;
            }
            video_source_->AddConsumer(xrtc_video_source_.get());
            AddMediaObject(xrtc_video_source_.get());
            AddMediaObject(x264_encoder_filter_.get());
            AddMediaObject(xrtc_media_sink_.get());
            if(!ConnectMediaObject(xrtc_video_source_.get(), x264_encoder_filter_.get())){
                err = XRTCError::kChainConnectErr;
                RTC_LOG(LS_WARNING) << "xrtc_video_source connect to x264_encoder_filter failed";
                break;
            }

            if(!ConnectMediaObject(x264_encoder_filter_.get(), xrtc_media_sink_.get())){
                err = XRTCError::kChainConnectErr;
                RTC_LOG(LS_WARNING) << "x264_encoder_filter connect to xrtc_media_sink failed";
                break;
            }


            // 安装参数
            JsonObject jobj;
            JsonObject j_xrtc_media_sink;
            j_xrtc_media_sink["url"] = pusher_->Url();
            jobj["xrtc_media_sink"] = j_xrtc_media_sink;
            SetupChain(JsonValue(jobj).ToJson());

            if(!StartChain()){
                err = XRTCError::kChainStartErr;
                RTC_LOG(LS_WARNING) << "XRTCPushStream Start failed, chain start failed";
                break;
            }

        }while(false);

        if(err != XRTCError::kNoErr){
            if(XRTCGlobal::Instance()->engine_observer())
                XRTCGlobal::Instance()->engine_observer()->OnPushFailed(pusher_, err);
        }
    }

    void XRTCPushStream::Stop() {

    }

    void XRTCPushStream::Destroy() {

    }

    void XRTCPushStream::OnChainSuccess() {
        if(XRTCGlobal::Instance()->engine_observer())
            XRTCGlobal::Instance()->engine_observer()->OnPushSuccess(pusher_);
    }

    void XRTCPushStream::OnChainFailed(MediaObject *, XRTCError err) {
        if(XRTCGlobal::Instance()->engine_observer())
            XRTCGlobal::Instance()->engine_observer()->OnPushFailed(pusher_, err);
    }
}