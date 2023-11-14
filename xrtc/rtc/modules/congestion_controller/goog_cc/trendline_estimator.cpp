#include "xrtc/rtc/modules/congestion_controller/goog_cc/trendline_estimator.h"

namespace xrtc {

xrtc::TrendlineEstimator::TrendlineEstimator() {
}

TrendlineEstimator::~TrendlineEstimator() {
}

void TrendlineEstimator::Update(webrtc::TimeDelta recv_time_delta, 
    webrtc::TimeDelta send_time_delta, 
    webrtc::Timestamp send_time, 
    webrtc::Timestamp arrival_time, 
    size_t packet_size, 
    bool calculated_delta) 
{
}

} // namespace xrtc
