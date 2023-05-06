//
// Created by faker on 2023/5/6.
//

#ifndef XRTCSDK_CAM_IMPL_H
#define XRTCSDK_CAM_IMPL_H

#include "xrtc/xrtc.h"
#include "rtc_base/thread.h"
#include "modules/video_capture/video_capture.h"

namespace xrtc{
    class CamImpl : public IVideoSource, public rtc::VideoSinkInterface<webrtc::VideoFrame>{
    public:
        virtual void Setup(const std::string& json_config) override;
        virtual void Start() override ;
        virtual void Stop() override ;
        virtual void Destroy() override ;
        virtual void AddConsumer(IXRTCConsumer* consumer) override ;
        virtual void RemoveConsumer(IXRTCConsumer* consumer) override ;

        // rtc::VideoSinkInterface
        virtual void OnFrame(const webrtc::VideoFrame& frame) override;
    private:
        CamImpl(const std::string& cam_id);
        ~CamImpl();
        friend XRTCEngine;
    private:
        std::string cam_id_;
        rtc::Thread* current_thread_;
//        std::vector<IXRTCConsumer* > consumers_;
        bool has_start_ = false;
        rtc::scoped_refptr<webrtc::VideoCaptureModule> video_capture_;
        webrtc::VideoCaptureModule::DeviceInfo* device_info_;
    };
}


#endif //XRTCSDK_CAM_IMPL_H
