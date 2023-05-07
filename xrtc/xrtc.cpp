#include "xrtc/xrtc.h"
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_global.h"
#include "xrtc/device/cam_impl.h"

namespace xrtc {
    void XRTCEngine::Init(XRTCEngineObserver *observer) {
        rtc::LogMessage::LogTimestamps(true);
        rtc::LogMessage::LogThreads(true);
        rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
        XRTCGlobal::Instance()->RegisterEngineObserver(observer);

        RTC_LOG(LS_INFO) << "XRTCSDK init";
    }

    uint32_t XRTCEngine::GetCameraCount() {
        return XRTCGlobal::Instance()->api_thread()->Invoke<uint32_t>(RTC_FROM_HERE, [=]() {
            return XRTCGlobal::Instance()->video_device_info()->NumberOfDevices();
        });
    }

    int32_t XRTCEngine::GetCameraInfo(int index, std::string &device_name, std::string &device_id) {

        return XRTCGlobal::Instance()->api_thread()->Invoke<int32_t>(RTC_FROM_HERE, [&]() {
            char name[256];
            char id[256];
            int32_t ret = XRTCGlobal::Instance()->video_device_info()->GetDeviceName(index, name, sizeof(name), id,
                                                                                     sizeof(id));

            device_name = name;
            device_id = id;
            return ret;
        });
    }


    IVideoSource* XRTCEngine::CreateCamSource(const std::string& cam_id){
        return XRTCGlobal::Instance()->api_thread()->Invoke<IVideoSource*>(RTC_FROM_HERE,[=](){
            return new CamImpl(cam_id);
        });
    }
}
