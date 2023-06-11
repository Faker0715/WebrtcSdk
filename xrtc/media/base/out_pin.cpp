//
// Created by faker on 2023/6/11.
//

#include "out_pin.h"
#include "in_pin.h"
namespace xrtc{

    OutPin::OutPin(MediaObject *obj) : BasePin(obj) {

    }
    void OutPin::PushMediaFrame(std::shared_ptr<MediaFrame> frame) {
        if(in_pin_){
            in_pin_->PushMediaFrame(frame);
        }
    }

    OutPin::~OutPin() {

    }

    bool OutPin::ConnectTo(InPin *in_pin) {
        if(!in_pin || !in_pin->Accept(this)){
            return false;
        }
        in_pin_ = in_pin;
        return true;
    }
}
