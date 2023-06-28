//
// Created by faker on 2023/5/5.
//
#include "xrtc/base/xrtc_global.h"
#include <modules/video_capture/video_capture_factory.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_http.h"

namespace xrtc {

    XRTCGlobal *XRTCGlobal::Instance() {
        // 第一次调用会被初始化
        static XRTCGlobal *const instance = new XRTCGlobal();
        return instance;
    }

    XRTCGlobal::XRTCGlobal() : api_thread_(rtc::Thread::Create()),
                               worker_thread_(rtc::Thread::Create()),
                               network_thread_(rtc::Thread::CreateWithSocketServer()),
                               task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
                               video_device_info_(webrtc::VideoCaptureFactory::CreateDeviceInfo()) {
        api_thread_->SetName("api_thread", nullptr);
        api_thread_->Start();
        worker_thread_->SetName("worker_thread", nullptr);
        worker_thread_->Start();
        network_thread_->SetName("network_thread", nullptr);
        network_thread_->Start();

        http_manager_ = new HttpManager();
        http_manager_->Start();
        ice::NetworkConfig config;
        port_allocator_ = std::make_unique<ice::PortAllocator>(config);

        // 在api_thread创建并初始化音频设备
        api_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            audio_device_ = webrtc::AudioDeviceModule::Create(
                    webrtc::AudioDeviceModule::kPlatformDefaultAudio,
                    task_queue_factory_.get());
            audio_device_->Init();
        }));

    }

    XRTCGlobal::~XRTCGlobal() {

    }
}