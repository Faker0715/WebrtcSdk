//
// Created by faker on 2023/5/6.
//

#include "cam_impl.h"
#include "rtc_base/task_utils/to_queued_task.h"
#include "modules/video_capture/video_capture_factory.h"
#include "xrtc/base/xrtc_global.h"
#include <rtc_base/logging.h>
#include <rtc_base/time_utils.h>

namespace xrtc {

    void CamImpl::Start() {
        RTC_LOG(LS_INFO) << "CamImpl::Start call";

        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {

            RTC_LOG(LS_INFO) << "CamImpl::Start PostTask";
            XRTCError err = XRTCError::kNoErr;
            do {
                if (has_start_) {
                    RTC_LOG(LS_WARNING) << "CamImpl::Start has start, ignore";
                }
                video_capture_ = webrtc::VideoCaptureFactory::Create(cam_id_.c_str());
                if(!video_capture_){
                    err = XRTCError::kVideoCaptureCreateErr;
                    RTC_LOG(LS_WARNING) << "CamImpl::Start Create video capture error";
                    break;
                }
                // 判断摄像头采集能力
                if(device_info_->NumberOfCapabilities(cam_id_.c_str()) <= 0){
                    err = XRTCError::kVideoNoCapabilitiesErr;
                    RTC_LOG(LS_WARNING) << "CamImpl::Start No capability";
                    break;
                }


                // 获取最佳的摄像头能力
                webrtc::VideoCaptureCapability request_cap;
                request_cap.width = 640;
                request_cap.height = 480;
                request_cap.maxFPS = 20;

                webrtc::VideoCaptureCapability bset_cap;
                if(device_info_->GetBestMatchedCapability(cam_id_.c_str(),
                                                          request_cap,bset_cap) < 0){
                    err = XRTCError::kVideoNoBestCapabilitiesErr;
                    RTC_LOG(LS_WARNING) << "CamImpl::Start No best capability";
                    break;
                }

                video_capture_->RegisterCaptureDataCallback(this);
                // 启动摄像头采集
                if(video_capture_->StartCapture(bset_cap) < 0){
                    err = XRTCError::kVideoStartCaptureErr;
                    RTC_LOG(LS_WARNING) << "CamImpl::Start Start capture error";
                    break;
                }
                has_start_ = true;
            } while (0);
        }));

    }

    void CamImpl::Stop() {

        RTC_LOG(LS_INFO) << "CamImpl Stop call";
        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            RTC_LOG(LS_INFO) << "CamImpl Stop PostTask";
            if(!has_start_){
                return;
            }
            if(video_capture_ && video_capture_->CaptureStarted()){
                video_capture_->StopCapture();
                video_capture_ = nullptr;
            }
            has_start_ = false;
        }));
    }

    void CamImpl::Destroy() {
        RTC_LOG(LS_INFO) << "CamImpl Destroy call";
        current_thread_->PostTask(webrtc::ToQueuedTask([=]() {
            RTC_LOG(LS_INFO) << "CamImpl Destroy PostTask";
            delete this;
        }));
    }

    CamImpl::CamImpl(const std::string &cam_id) : cam_id_(cam_id),
            // 哪个现成调用的就是哪个线程
                                                  current_thread_(rtc::Thread::Current()),
                                                  device_info_(XRTCGlobal::Instance()->video_device_info()){

    }

    void CamImpl::AddConsumer(IXRTCConsumer *consumer) {

    }

    void CamImpl::RemoveConsumer(IXRTCConsumer *consumer) {

    }

    CamImpl::~CamImpl() {


    }

    void CamImpl::Setup(const std::string &json_config) {

    }

    void CamImpl::OnFrame(const webrtc::VideoFrame &frame) {
        if(0 == last_frame_ts_){
            last_frame_ts_ = rtc::Time();
        }
        fps_++;
        int64_t now = rtc::Time();
        if(now - last_frame_ts_ > 1000){
            RTC_LOG(LS_INFO) << "=====fps: " << fps_;
            fps_ = 0;
            last_frame_ts_ = now;
        }
    }
}