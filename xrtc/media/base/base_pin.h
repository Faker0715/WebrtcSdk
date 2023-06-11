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
            fmt = format;
        }
        MediaFormat format() const{
            return fmt;
        }

        virtual void PushMediaFrame(std::shared_ptr<MediaFrame> frame) = 0;

    private:
        MediaObject* obj_;
        MediaFormat fmt;
    };

}


#endif //XRTCSDK_BASE_PIN_H
