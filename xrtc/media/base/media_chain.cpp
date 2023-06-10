//
// Created by faker on 2023/6/10.
//

#include "media_chain.h"
namespace xrtc{
    MediaChain::~MediaChain() {
        for (auto& media_object : media_objects_) {
            delete media_object;
        }
        media_objects_.clear();
    }

    void MediaChain::AddMediaObject(MediaObject *media_object) {
        media_objects_.push_back(media_object);
    }
}
