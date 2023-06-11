//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_BASE_PIN_H
#define XRTCSDK_BASE_PIN_H

#include <memory>
#include "xrtc/media/media_frame.h"

namespace xrtc{
    class MediaObject;
    class BasePin{
    public:
        BasePin(MediaObject* obj):obj_(obj){};
        virtual ~BasePin(){};
        MediaObject* GetMediaObject() const{
            return obj_;
        }
        void set_format(const MediaFormat& format){
            fmt_ = format;
        }
        MediaFormat format() const{
            return fmt_;
        }

        virtual void PushMediaFrame(std::shared_ptr<MediaFrame> frame) = 0;

    protected:
        MediaObject* obj_;
        MediaFormat fmt_;
    };

}


#endif //XRTCSDK_BASE_PIN_H
