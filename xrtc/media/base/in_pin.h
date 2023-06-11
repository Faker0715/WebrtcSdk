//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_IN_PIN_H
#define XRTCSDK_IN_PIN_H

#include "base_pin.h"

namespace xrtc{
    class OutPin;
    class InPin : public BasePin{
    public:
        InPin() = delete;
        explicit InPin(MediaObject* obj);
        ~InPin() override;
        bool Accept(OutPin* out_pin);
        void PushMediaFrame(std::shared_ptr<MediaFrame> frame) override;
    private:
        OutPin* out_pin_ = nullptr;
    };
}


#endif //XRTCSDK_IN_PIN_H
