//
// Created by faker on 2023/6/11.
//

#include "in_pin.h"
namespace xrtc{

    InPin::InPin(MediaObject *obj) : BasePin(obj) {

    }

    bool InPin::Accept(OutPin *out_pin) {
        return false;
    }

    void InPin::PushMediaFrame(std::shared_ptr<MediaFrame> frame) {

    }

    InPin::~InPin() {

    }
}
