//
// Created by faker on 2023/6/16.
//

#ifndef XRTCSDK_XRTC_PUSH_STREAM_H
#define XRTCSDK_XRTC_PUSH_STREAM_H

#include "xrtc/media/base/media_chain.h"
#include "xrtc/media/source/xrtc_video_source.h"
#include "xrtc/media/filter/x264_encoder_filter.h"
#include "xrtc/media/sink/xrtc_media_sink.h"

namespace xrtc {
    class XRTCPusher;
    class XRTCPushStream : public MediaChain {
    public:
        XRTCPushStream(XRTCPusher* pusher, IVideoSource* video_source);
        ~XRTCPushStream() override;
        void Start() override;
        void Stop() override;
        void Destroy() override;
    private:
        XRTCPusher* pusher_;
        IVideoSource* video_source_;
        std::unique_ptr<XRTCVideoSource> xrtc_video_source_;
        std::unique_ptr<X264EncoderFilter> x264_encoder_filter_;
        std::unique_ptr<XRTCMediaSink> xrtc_media_sink_;
    };
}


#endif //XRTCSDK_XRTC_PUSH_STREAM_H
