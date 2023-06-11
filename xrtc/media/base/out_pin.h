//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_OUT_PIN_H
#define XRTCSDK_OUT_PIN_H

#include "base_pin.h"

namespace xrtc{
    class InPin;
    class OutPin : public BasePin{
    public:
        OutPin() = delete;
        explicit OutPin(MediaObject* obj);
        ~OutPin() override;
        bool ConnectTo(InPin* in_pin);
        void PushMediaFrame(std::shared_ptr<MediaFrame> frame) override;
    private:
        InPin* in_pin_ = nullptr;
    };
}


#endif //XRTCSDK_OUT_PIN_H
