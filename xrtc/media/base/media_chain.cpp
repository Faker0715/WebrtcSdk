//
// Created by faker on 2023/6/10.
//

#include "media_chain.h"
namespace xrtc{
    void MediaChain::AddMediaObject(MediaObject *media_object) {
        media_objects_.push_back(media_object);
    }
}
