#include "xrtc/rtc/modules/congestion_controller/goog_cc/trendline_estimator.h"

namespace xrtc {
    namespace {

        const double kDefaultTrendlineSmoothingCoef = 0.9;
        const size_t kDefaultTrendlineWindowSize = 20;

    } // namespace

    xrtc::TrendlineEstimator::TrendlineEstimator() :
            smoothing_coef_(kDefaultTrendlineSmoothingCoef)
    {
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
        if (calculated_delta) {
            UpdateTrendline(recv_time_delta.ms<double>(),
                            send_time_delta.ms<double>(),
                            send_time.ms(), arrival_time.ms(),
                            packet_size);
        }
    }

    void TrendlineEstimator::UpdateTrendline(double recv_delta_ms,
                                             double send_delta_ms,
                                             int64_t send_time_ms,
                                             int64_t arrival_time_ms,
                                             size_t packet_size)
    {
        if (-1 == first_arrival_time_ms_) {
            first_arrival_time_ms_ = arrival_time_ms;
        }

        // 计算传输的延迟差
        double delta_ms = recv_delta_ms - send_delta_ms;
        accumulated_delay_ms_ += delta_ms;
        // 计算指数平滑后的累计延迟差
        smoothed_delay_ms_ = smoothing_coef_ * smoothed_delay_ms_ +
                             (1 - smoothing_coef_) * accumulated_delay_ms_;
        // 将样本数据添加到队列
        delay_hist_.emplace_back(
                static_cast<double>(arrival_time_ms - first_arrival_time_ms_),
                smoothed_delay_ms_, accumulated_delay_ms_);

        if (delay_hist_.size() > 20) {
            delay_hist_.pop_front();
        }

        // 当样本数据满足要求，计算trend值
        double trend = prev_trend_;
        if (delay_hist_.size() == kDefaultTrendlineWindowSize) {
            trend = LinearFitSlope(delay_hist_).value_or(trend);
        }
    }

    absl::optional<double> TrendlineEstimator::LinearFitSlope(
            const std::deque<PacketTiming>& packets)
    {
        return absl::optional<double>();
    }

} // namespace xrtc
