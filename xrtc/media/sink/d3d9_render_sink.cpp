//
// Created by faker on 2023/6/11.
//

#include "d3d9_render_sink.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/base/xrtc_global.h"
#include "rtc_base/task_utils/to_queued_task.h"

#include "d3d9.h"
#include "rtc_base/logging.h"

namespace xrtc {

    D3D9RenderSink::D3D9RenderSink() :
            in_pin_(std::make_unique<InPin>(this)) {
        MediaFormat fmt;
        fmt.media_type = MainMediaType::kMainTypeVideo;
        fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
        in_pin_->set_format(fmt);
    }

    D3D9RenderSink::~D3D9RenderSink() {

    }

    bool D3D9RenderSink::Start() {
        return true;
    }

    void D3D9RenderSink::Stop() {

    }

    void D3D9RenderSink::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        // worker_thread执行渲染工作
        XRTCGlobal::Instance()->worker_thread()->PostTask(webrtc::ToQueuedTask([=]() {
            // 如果d3d9没有渲染成功 就不能进行后续的工作
            if (!TryInit(frame)) {

                return;
            }

        }));


    }

    void D3D9RenderSink::SetUp(const std::string &json_config) {
        JsonValue value;
        value.FromJson(json_config);
        JsonObject jobject = value.ToObject();
        JsonObject jd3d9 = jobject["d3d9_render_sink"].ToObject();
        hwnd_ = (HWND) jd3d9["hwnd"].ToInt();

    }


    bool D3D9RenderSink::TryInit(std::shared_ptr<MediaFrame> frame) {
        do{
            if(!d3d9_ || !d3d9_device_ || !d3d9_surface_){
                break;
            }
            // 如果屏幕发生了变化
            if(width_ != frame->fmt.sub_fmt.video_fmt.width || height_ != frame->fmt.sub_fmt.video_fmt.height){
                break;
            }
            return true;
        }while (false);
        if (!IsWindow(hwnd_)) {
            RTC_LOG(LS_WARNING) << "D3D9RenderSink::TryInit hwnd is invalid";
            return false;
        }
        if (!d3d9_) {
            d3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
            if (!d3d9_) {
                RTC_LOG(LS_WARNING) << "D3D9RenderSink::TryInit Direct3DCreate9 failed";
                return false;
            }
        }
        if (!d3d9_device_) {
            // 如果没有创建过device 就创建一个
            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));
            d3dpp.Windowed = TRUE;
            d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

            HRESULT res = d3d9_->CreateDevice(
                    D3DADAPTER_DEFAULT, // 指定要表示的物理设备，默认主显示器
                    D3DDEVTYPE_HAL, // 指定要创建的设备类型，D3DDEVTYPE_HAL表示使用硬件加速
                    hwnd_, // 指定要渲染的窗口句柄
                    D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, // 指定设备创建的行为
                    &d3dpp,
                    &d3d9_device_);
            if (FAILED(res)) {
                RTC_LOG(LS_WARNING) << "D3D9RenderSink::TryInit CreateDevice failed";
                return false;
            }
        }

        // 3. 创建离屏表面
        if (d3d9_surface_) {
            d3d9_surface_->Release();
            d3d9_surface_ = nullptr;
        }
        HRESULT res = d3d9_device_->CreateOffscreenPlainSurface(
                frame->fmt.sub_fmt.video_fmt.width,
                frame->fmt.sub_fmt.video_fmt.height,
                D3DFMT_X8R8G8B8, // 指定表面的格式
                D3DPOOL_DEFAULT,
                &d3d9_surface_,
                NULL);
        if (FAILED(res)) {
            RTC_LOG(LS_WARNING) << "D3D9RenderSink::TryInit CreateOffscreenPlainSurface failed";
            return false;
        }
        width_ = frame->fmt.sub_fmt.video_fmt.width;
        height_ = frame->fmt.sub_fmt.video_fmt.height;


        return true;
    }
}