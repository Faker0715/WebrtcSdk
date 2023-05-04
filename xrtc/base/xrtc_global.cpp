//
// Created by faker on 2023/5/5.
//
#include "xrtc/base/xrtc_global.h"

namespace xrtc {

    XRTCGlobal *XRTCGlobal::Instance() {
        // 第一次调用会被初始化
        static XRTCGlobal *const instance = new XRTCGlobal();
        return instance;
    }

    XRTCGlobal::XRTCGlobal() : api_thread_(rtc::Thread::CreateWithSocketServer()),
                               worker_thread_(rtc::Thread::Create()),
                               network_thread_(rtc::Thread::CreateWithSocketServer()) {
        api_thread_->SetName("api_thread", nullptr);
        api_thread_->Start();
        worker_thread_->SetName("worker_thread", nullptr);
        worker_thread_->Start();
        network_thread_->SetName("network_thread", nullptr);
        network_thread_->Start();

    }

    XRTCGlobal::~XRTCGlobal() {

    }
}