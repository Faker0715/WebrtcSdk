//
// Created by faker on 2023/5/5.
//

#ifndef XRTCSDK_XRTC_GLOBAL_H
#define XRTCSDK_XRTC_GLOBAL_H
#include <rtc_base/thread.h>
#include <modules/video_capture/video_capture.h>
namespace xrtc {
    class XRTCGlobal {
    public:
        static XRTCGlobal* Instance();
        rtc::Thread* api_thread() const { return api_thread_.get(); }
        rtc::Thread* worker_thread() const { return worker_thread_.get(); }
        rtc::Thread* network_thread() const { return network_thread_.get(); }
        webrtc::VideoCaptureModule::DeviceInfo* video_device_info() const { return video_device_info_.get(); }

    private:
        XRTCGlobal();
        ~XRTCGlobal();
    private:
        std::unique_ptr<rtc::Thread> api_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_device_info_;

    };
}
#endif //XRTCSDK_XRTC_GLOBAL_H