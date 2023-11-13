#include "xrtc/rtc/modules/congestion_controller/goog_cc/delay_based_bwe.h"

namespace xrtc {

DelayBasedBwe::DelayBasedBwe() {
}

DelayBasedBwe::~DelayBasedBwe() {
}

DelayBasedBwe::Result DelayBasedBwe::IncomingPacketFeedbackVector(
    const webrtc::TransportPacketsFeedback& msg) 
{
    return DelayBasedBwe::Result();
}

} // namespace xrtc