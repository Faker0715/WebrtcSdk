#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_

#include <api/units/time_delta.h>
#include <api/units/timestamp.h>

namespace xrtc {

class TrendlineEstimator {
public:
    TrendlineEstimator();
    ~TrendlineEstimator();

    void Update(webrtc::TimeDelta recv_time_delta,
        webrtc::TimeDelta send_time_delta,
        webrtc::Timestamp send_time,
        webrtc::Timestamp arrival_time,
        size_t packet_size,
        bool calculated_delta);
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_