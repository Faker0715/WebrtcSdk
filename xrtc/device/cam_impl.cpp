//
// Created by faker on 2023/5/6.
//

#include "cam_impl.h"
namespace xrtc{

    void CamImpl::Start() {

    }

    void CamImpl::Stop() {

    }

    void CamImpl::Destroy() {

    }

    CamImpl::CamImpl(const std::string &cam_id):cam_id_(cam_id),
            // 哪个现成调用的就是哪个线程
             current_thread_(rtc::Thread::Current()){

    }

    void CamImpl::AddConsumer(IXRTCConsumer *consumer) {

    }

    void CamImpl::RemoveConsumer(IXRTCConsumer *consumer) {

    }

    CamImpl::~CamImpl() {

    }
}