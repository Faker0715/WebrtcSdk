//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_D3D9_RENDER_SINK_H
#define XRTCSDK_D3D9_RENDER_SINK_H

#include <windows.h>
#include "xrtc/media/base/media_chain.h"
#include "xrtc/base/xrtc_json.h"

namespace xrtc{

    class InPin;
    class OutPin;
    class D3D9RenderSink : public MediaObject{
    public:
        D3D9RenderSink();
        ~D3D9RenderSink() override;

        bool Start() override;
        void Stop() override;
        void SetUp(const std::string& json_config) override;
        virtual std::vector<InPin*> GetAllInPins() override{
            return std::vector<InPin*>({in_pin_.get()});
        }
        virtual std::vector<OutPin*> GetAllOutPins() override{
            return std::vector<OutPin*>();
        }
        void OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) override;
    private:
        std::unique_ptr<InPin> in_pin_;
        HWND hwnd_;
    };


}


#endif //XRTCSDK_D3D9_RENDER_SINK_H
