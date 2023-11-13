#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_

#include "api/transport/network_types.h"
#include "api/units/data_rate.h"

namespace xrtc {

class DelayBasedBwe {
public:
    struct Result {
        bool updated = false;
        webrtc::DataRate target_bitrate = webrtc::DataRate::Zero();
    };

    DelayBasedBwe();
    virtual ~DelayBasedBwe();

    Result IncomingPacketFeedbackVector(
        const webrtc::TransportPacketsFeedback& msg);
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_