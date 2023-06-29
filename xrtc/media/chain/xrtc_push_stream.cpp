//
// Created by faker on 2023/6/16.
//

#include "xrtc_push_stream.h"
#include "rtc_base/logging.h"
#include "xrtc/base/xrtc_json.h"
#include "xrtc/media/base/xrtc_pusher.h"
#include "xrtc/base/xrtc_global.h"
namespace xrtc{

    XRTCPushStream::XRTCPushStream(XRTCPusher* pusher, IAudioSource* audio_source, IVideoSource* video_source) :
            pusher_(pusher),
            audio_source_(audio_source),
            video_source_(video_source),
            xrtc_audio_source_(std::make_unique<XRTCAudioSource>()),
            xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
            audio_processing_filter_(std::make_unique<AudioProcessingFilter>()),
            x264_encoder_filter_(std::make_unique<X264EncoderFilter>()),
            opus_encoder_filter_(std::make_unique<OpusEncoderFilter>()),
            xrtc_media_sink_(std::make_unique<XRTCMediaSink>(this))
    {
    }


    XRTCPushStream::~XRTCPushStream() {

    }

    void XRTCPushStream::Start() {
        RTC_LOG(LS_INFO) << "XRTCPushStream Start";

        XRTCError err = XRTCError::kNoErr;

        do {
            if (!audio_source_ && !video_source_) {
                err = XRTCError::kPushNoMediaSourceErr;
                RTC_LOG(LS_WARNING) << "PushStream start failed: no media source";
                break;
            }

            if (audio_source_) {
                audio_source_->AddConsumer(xrtc_audio_source_.get());
                AddMediaObject(audio_processing_filter_.get());
                AddMediaObject(opus_encoder_filter_.get());
            }

            if (video_source_) {
                video_source_->AddConsumer(xrtc_video_source_.get());
                AddMediaObject(xrtc_video_source_.get());
                AddMediaObject(x264_encoder_filter_.get());
            }

            AddMediaObject(xrtc_media_sink_.get());

            if (audio_source_) {
                if (!ConnectMediaObject(xrtc_audio_source_.get(), audio_processing_filter_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "xrtc_audio_source connect to audio_processing_filter failed";
                    break;
                }

                if (!ConnectMediaObject(audio_processing_filter_.get(), opus_encoder_filter_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "audio_processing_filter_ connect to opus_encoder_filter_ failed";
                    break;
                }

                if (!ConnectMediaObject(opus_encoder_filter_.get(), xrtc_media_sink_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "opus_encoder_filter connect to xrtc_media_sink failed";
                    break;
                }
            }

            if (video_source_) {
                if (!ConnectMediaObject(xrtc_video_source_.get(), x264_encoder_filter_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "xrtc_video_source connect to x264_encoder_filter failed";
                    break;
                }

                if (!ConnectMediaObject(x264_encoder_filter_.get(), xrtc_media_sink_.get())) {
                    err = XRTCError::kChainConnectErr;
                    RTC_LOG(LS_WARNING) << "x264_encoder_filter connect to xrtc_media_sink failed";
                    break;
                }
            }

            // 安装参数
            JsonObject jobj;

            JsonObject j_audio_processing_filter;
            j_audio_processing_filter["AEC"] = true;
            j_audio_processing_filter["VAD"] = true;
            j_audio_processing_filter["high_pass_filter"] = true;
            j_audio_processing_filter["ANS"] = true;
            j_audio_processing_filter["AGC"] = true;
            j_audio_processing_filter["encoder_clock_rate"] = 48000;
            j_audio_processing_filter["encoder_channels"] = 2;

            JsonObject j_xrtc_media_sink;
            j_xrtc_media_sink["url"] = pusher_->Url();
            jobj["audio_processing_filter"] = j_audio_processing_filter;
            jobj["xrtc_media_sink"] = j_xrtc_media_sink;

            SetupChain(JsonValue(jobj).ToJson());

            if (!StartChain()) {
                err = XRTCError::kChainStartErr;
                RTC_LOG(LS_WARNING) << "PushStream StartChain failed";
                break;
            }

        } while (false);

        if (err != XRTCError::kNoErr) {
            if (XRTCGlobal::Instance()->engine_observer()) {
                XRTCGlobal::Instance()->engine_observer()->OnPushFailed(pusher_, err);
            }
        }
    }


    void XRTCPushStream::Stop() {
        // TODO: 如果先停止设备 再停止推流 此处crash
        if(audio_source_){
            audio_source_->RemoveConsumer(xrtc_audio_source_.get());
        }
        if(video_source_){
            video_source_->RemoveConsumer(xrtc_video_source_.get());
        }

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