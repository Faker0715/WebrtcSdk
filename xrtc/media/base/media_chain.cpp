//
// Created by faker on 2023/6/10.
//

#include "media_chain.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/media/base/out_pin.h"

namespace xrtc {
    void MediaChain::AddMediaObject(MediaObject *media_object) {
        media_objects_.push_back(media_object);
    }

    bool MediaChain::ConnectMediaObject(MediaObject *from, MediaObject *to) {
        if (!from || !to) {
            return false;
        }
        std::vector<OutPin *> out_pins = from->GetAllOutPins();
        std::vector<InPin *> in_pins = to->GetAllInPins();
        for(auto out_pin : out_pins){
            bool has_connected = false;
            for(auto in_pin : in_pins){
                if(out_pin->ConnectTo(in_pin)){
                    has_connected = true;
                    break;
                }
            }
            if(!has_connected){
                return false;
            }
        }
        return true;

    }

    bool MediaChain::StartChain() {
        for(auto media_object : media_objects_){
            if(!media_object->Start()){
                return false;
            }
        }
        return true;
    }

    void MediaChain::SetupChain(const std::string &json_config) {
        for(auto obj: media_objects_){
            obj->SetUp(json_config);
        }

    }
}
