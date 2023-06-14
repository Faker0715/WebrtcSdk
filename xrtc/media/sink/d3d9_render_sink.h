//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_D3D9_RENDER_SINK_H
#define XRTCSDK_D3D9_RENDER_SINK_H

#include <windows.h>
#include "xrtc/media/base/media_chain.h"
#include "xrtc/base/xrtc_json.h"

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;

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
        bool TryInit(std::shared_ptr<MediaFrame> frame);
        void DoRender(std::shared_ptr<MediaFrame> frame);
    private:
        std::unique_ptr<InPin> in_pin_;
        HWND hwnd_;
        IDirect3D9* d3d9_ = nullptr;
        IDirect3DDevice9* d3d9_device_ = nullptr;
        IDirect3DSurface9* d3d9_surface_ = nullptr;
        int width_ = 640;
        int height_ = 480;
        char* rgb_buffer_ = nullptr;
        int rgb_buffer_size_ = 0;
    };


}


#endif //XRTCSDK_D3D9_RENDER_SINK_H
