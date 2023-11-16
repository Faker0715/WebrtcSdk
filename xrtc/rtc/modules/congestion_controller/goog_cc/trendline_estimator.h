#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_

#include <deque>

#include <absl/types/optional.h>
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

        struct PacketTiming {
            PacketTiming(double arrival_time_ms,
                         double smoothed_delay_ms,
                         double raw_delay_ms) :
                    arrival_time_ms(arrival_time_ms),
                    smoothed_delay_ms(smoothed_delay_ms),
                    raw_delay_ms(raw_delay_ms) { }

            double arrival_time_ms; // 数据包到达的时间
            double smoothed_delay_ms; // 指数平滑之后的延迟差
            double raw_delay_ms; // 原始的延迟差
        };

    private:
        void UpdateTrendline(double recv_delta_ms,
                             double send_delta_ms,
                             int64_t send_time_ms,
                             int64_t arrival_time_ms,
                             size_t packet_size);

        absl::optional<double> LinearFitSlope(
                const std::deque<PacketTiming>& packets);

    private:
        int64_t first_arrival_time_ms_ = -1;
        double accumulated_delay_ms_ = 0;
        double smoothed_delay_ms_ = 0;
        // 表示了历史数据的权重
        double smoothing_coef_;
        std::deque<PacketTiming> delay_hist_;
        double prev_trend_ = 0.0f;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_