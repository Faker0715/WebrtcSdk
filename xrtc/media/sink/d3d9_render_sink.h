//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_D3D9_RENDER_SINK_H
#define XRTCSDK_D3D9_RENDER_SINK_H

#include "xrtc/media/base/media_chain.h"

namespace xrtc{
    class D3D9RenderSink : public MediaObject{
    public:
        D3D9RenderSink();
        ~D3D9RenderSink() override;

        bool Start() override;
        void Stop() override;
    };


}


#endif //XRTCSDK_D3D9_RENDER_SINK_H
