#include "xrtc/xrtc.h"
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_global.h"
namespace xrtc {
    void XRTCEngine::Init() {
//        XRTCGlobal::Instance()->api_thread()->Invoke<void>(RTC_FROM_HERE,[=](){
//            int a = 1;
//        });
        XRTCGlobal::Instance()->api_thread()->PostTask(webrtc::ToQueuedTask([=](){
            int a = 1;
        }));
        int b = 1;
    }
}
