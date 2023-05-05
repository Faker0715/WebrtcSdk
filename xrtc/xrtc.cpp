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

    }

    uint32_t XRTCEngine::GetCameraCount() {
        return XRTCGlobal::Instance()->api_thread()->Invoke<uint32_t>(RTC_FROM_HERE,[=](){
            return XRTCGlobal::Instance()->video_device_info()->NumberOfDevices();
        });
    }
}
