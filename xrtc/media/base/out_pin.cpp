//
// Created by faker on 2023/6/11.
//

#include "out_pin.h"
namespace xrtc{

    OutPin::OutPin(MediaObject *obj) : BasePin(obj) {

    }
    void OutPin::PushMediaFrame(std::shared_ptr<MediaFrame> frame) {

    }

    OutPin::~OutPin() {

    }

    bool OutPin::ConnectTo(InPin *in_pin) {
        return false;
    }
}
