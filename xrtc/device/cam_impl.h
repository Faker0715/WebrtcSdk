//
// Created by faker on 2023/5/6.
//

#ifndef XRTCSDK_CAM_IMPL_H
#define XRTCSDK_CAM_IMPL_H

#include "xrtc/xrtc.h"
#include "rtc_base/thread.h"

namespace xrtc{
    class CamImpl : public IVideoSource{
    public:
        virtual void Start() override ;
        virtual void Stop() override ;
        virtual void Destroy() override ;
        virtual void AddConsumer(IXRTCConsumer* consumer) override ;
        virtual void RemoveConsumer(IXRTCConsumer* consumer) override ;
    private:
        CamImpl(const std::string& cam_id);
        ~CamImpl();
        friend XRTCEngine;
    private:
        std::string cam_id_;
        rtc::Thread* current_thread_;
        std::vector<IXRTCConsumer* > consumers_;
    };
}


#endif //XRTCSDK_CAM_IMPL_H
