#include "xrtc/xrtc.h"
#include <rtc_base/logging.h>
namespace xrtc {
    void test(){
        rtc::LogMessage::ConfigureLogging("thread");
    }

}
