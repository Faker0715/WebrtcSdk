//
// Created by faker on 2023/6/16.
//

#include "xrtc_push_stream.h"
#include "rtc_base/logging.h"

namespace xrtc{

    XRTCPushStream::XRTCPushStream(XRTCPusher *pusher, IVideoSource *video_source):
            pusher_(pusher),
            video_source_(video_source),
            xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
            x264_encoder_filter_(std::make_unique<X264EncoderFilter>()){

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
            if(!ConnectMediaObject(xrtc_video_source_.get(), x264_encoder_filter_.get())){
                err = XRTCError::kChainConnectErr;
                RTC_LOG(LS_WARNING) << "xrtc_video_source connect to x264_encoder_filter failed";
                break;
            }
            if(!StartChain()){
                err = XRTCError::kChainStartErr;
                RTC_LOG(LS_WARNING) << "XRTCPushStream Start failed, chain start failed";
                break;
            }

        }while(false);
    }

    void XRTCPushStream::Stop() {

    }

    void XRTCPushStream::Destroy() {

    }
}