#include "xrtc/xrtc.h"
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_global.h"

namespace xrtc {
    void XRTCEngine::Init() {
//        XRTCGlobal::Instance()->api_thread()->Invoke<void>(RTC_FROM_HERE,[=](){
//            int a = 1;
//        });
//        XRTCGlobal::Instance()->api_thread()->PostTask(webrtc::ToQueuedTask([=](){
//            int a = 1;
//        }));
        rtc::LogMessage::LogTimestamps(true);
        rtc::LogMessage::LogThreads(true);
        rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
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
}
