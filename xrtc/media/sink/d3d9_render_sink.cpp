//
// Created by faker on 2023/6/11.
//

#include "d3d9_render_sink.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/base/xrtc_global.h"
#include "rtc_base/task_utils/to_queued_task.h"
#include "libyuv.h"

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
            DoRender(frame);

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
        do {
            if (!d3d9_ || !d3d9_device_ || !d3d9_surface_) {
                break;
            }
            // 如果屏幕发生了变化
            if (width_ != frame->fmt.sub_fmt.video_fmt.width || height_ != frame->fmt.sub_fmt.video_fmt.height) {
                break;
            }
            return true;
        } while (false);
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

    void D3D9RenderSink::DoRender(std::shared_ptr<MediaFrame> frame) {
        // 1. 创建RGB buffer，将yuv格式转换为rgb格式
        if (SubMediaType::kSubTypeI420 == frame->fmt.sub_fmt.video_fmt.type) {

            int size = frame->fmt.sub_fmt.video_fmt.width *
                       frame->fmt.sub_fmt.video_fmt.height * 4;
            if (rgb_buffer_size_ != size) {
                if (rgb_buffer_) {
                    delete[] rgb_buffer_;
                }

                rgb_buffer_ = new char[size];
                rgb_buffer_size_ = size;
            }

            // YUV格式转换成RGB
            libyuv::I420ToARGB((const uint8_t*)frame->data[0], frame->stride[0],
                               (const uint8_t*)frame->data[1], frame->stride[1],
                               (const uint8_t*)frame->data[2], frame->stride[2],
                               (uint8_t*)rgb_buffer_, width_ * 4,
                               width_, height_);
        }

        // 2. 将rgb buffer渲染到离屏表面
        // 2.1 锁定区域
        HRESULT res;
        D3DLOCKED_RECT d3d9_rect;
        res = d3d9_surface_->LockRect(&d3d9_rect, // 指向一个D3DLOCKED_RECT结构的指针，用于接收锁定矩形的信息
                                      NULL, // 指向一个RECT结构的指针，用于指定要锁定的矩形区域，如果为NULL，则锁定整个表面
                                      D3DLOCK_DONOTWAIT// 指定锁定表面时的行为，D3DLOCK_DISCARD表示不保留表面的内容
        );
        if (FAILED(res)) {
            RTC_LOG(LS_WARNING) << "D3D9RenderSink::DoRender LockRect failed: " << res;
            return;
        }
        // 2.2 拷贝数据
        // 锁定区域的地址
        byte *pdest = (byte *) d3d9_rect.pBits;
        // 锁定区域每一行的数据大小
        int stride = d3d9_rect.Pitch;

        if (SubMediaType::kSubTypeI420 == frame->fmt.sub_fmt.video_fmt.type) {
            int video_width = frame->fmt.sub_fmt.video_fmt.width;
            int video_height = frame->fmt.sub_fmt.video_fmt.height;
            int video_stride = video_width * 4;
            if (video_stride == stride) {
                memcpy(pdest, rgb_buffer_,  rgb_buffer_size_);
            } else if (video_stride < stride) {
                char* src = rgb_buffer_;
                // 每一行的数据大小不一样，需要逐行拷贝
                for (int i = 0; i < video_height; ++i) {
                    memcpy(pdest, src, video_stride);
                    pdest += stride;
                    src += video_stride;
                }
            }
            // stride > video_stride 就不需要拷贝了 可以进行截断处理
        }
        // 2.3 解锁
        d3d9_surface_->UnlockRect();

        // 3. 清除后台缓冲
        d3d9_device_->Clear(0,// 清除的矩形区域，如果为NULL，则清除整个后台缓冲区
                            NULL,// 指向一个D3DRECT结构的指针，用于指定要清除的矩形区域，如果为NULL，则清除整个后台缓冲区
                             D3DCLEAR_TARGET, // 清除渲染目标
                             D3DCOLOR_XRGB(30, 30, 30), // 清除后的颜色
                             1.0f,
                             0
                             );
        // 4. 将离屏表面的内容渲染到后台缓冲区
        d3d9_device_->BeginScene();
        // 获取后台缓冲表面
        IDirect3DSurface9 *pback_buffer = nullptr;
        d3d9_device_->GetBackBuffer(0, // 正在使用的交换链的索引
                                          0, // 用于指定要获取的表面的类型，如果只有一个后台缓冲区，则为D3DBACKBUFFER_TYPE_MONO
                                          D3DBACKBUFFER_TYPE_MONO, // 用于指定要获取的表面的类型，如果只有一个后台缓冲区，则为D3DBACKBUFFER_TYPE_MONO
                                          &pback_buffer // 用于接收指向后台缓冲区的指针
        );
        d3d9_device_->StretchRect(d3d9_surface_, // 指向源表面的指针
                                  NULL, // 指向源矩形区域的指针，如果为NULL，则表示整个表面
                                  pback_buffer, // 指向目标表面的指针
                                  NULL, // 指向目标矩形区域的指针，如果为NULL，则表示整个表面
                                  D3DTEXF_LINEAR// 指定纹理过滤器 线性插值 两个区域大小不一样 还有临近算法 但是效果不好
        );
        d3d9_device_->EndScene();
        // 5. 显示图像, 表面翻转
        d3d9_device_->Present(NULL, // 指向一个矩形区域的指针，用于指定要显示的矩形区域，如果为NULL，则显示整个客户区
                              NULL, // 指向一个矩形区域的指针，用于指定要更新的矩形区域，如果为NULL，则更新整个客户区
                              NULL, // 指向一个窗口句柄的指针，用于指定要显示的窗口，如果为NULL，则显示主窗口
                              NULL // 指向一个D3DPRESENT_PARAMETERS结构的指针，用于指定如何进行显示
        );
        // 6. 释放资源
        pback_buffer->Release();



    }
}