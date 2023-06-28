//
// Created by faker on 2023/5/5.
//

#ifndef XRTCSDK_XRTC_GLOBAL_H
#define XRTCSDK_XRTC_GLOBAL_H
#include <rtc_base/thread.h>
#include <modules/video_capture/video_capture.h>
#include <modules/audio_device/include/audio_device.h>
#include <ice/port_allocator.h>
#include <api/task_queue/default_task_queue_factory.h>
namespace xrtc {
    class XRTCEngineObserver;
    class HttpManager;
    class XRTCGlobal {
    public:
        static XRTCGlobal* Instance();

        XRTCEngineObserver* engine_observer() const { return engine_observer_; }
        void RegisterEngineObserver(XRTCEngineObserver* observer) { engine_observer_ = observer; }



        rtc::Thread* api_thread() const { return api_thread_.get(); }
        rtc::Thread* worker_thread() const { return worker_thread_.get(); }
        rtc::Thread* network_thread() const { return network_thread_.get(); }
        webrtc::VideoCaptureModule::DeviceInfo* video_device_info() const { return video_device_info_.get(); }
        HttpManager* http_manager() { return http_manager_; }
        ice::PortAllocator* port_allocator() { return port_allocator_.get(); }
        webrtc::AudioDeviceModule* audio_device() { return audio_device_.get(); }
    private:
        XRTCGlobal();
        ~XRTCGlobal();
    private:
        std::unique_ptr<rtc::Thread> api_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_device_info_;
        std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
        rtc::scoped_refptr<webrtc::AudioDeviceModule> audio_device_;
        XRTCEngineObserver* engine_observer_;
        HttpManager* http_manager_ = nullptr;
        std::unique_ptr<ice::PortAllocator> port_allocator_;


    };
}
#endif //XRTCSDK_XRTC_GLOBAL_H
