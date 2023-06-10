//
// Created by faker on 2023/6/10.
//

#ifndef XRTCSDK_XRTC_RENDER_H
#define XRTCSDK_XRTC_RENDER_H

namespace xrtc{
    class XRTCRender{
    public:
        ~XRTCRender() {};
        void* canvas(){
            return canvas_;
        }
    private:
        XRTCRender(void* canvas):canvas_(canvas){};

        friend class XRTCEngine;
    private:
        void* canvas_;
    };
}


#endif //XRTCSDK_XRTC_RENDER_H
