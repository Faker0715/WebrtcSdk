#include "xrtc/rtc/modules/congestion_controller/goog_cc/trendline_estimator.h"

#include <rtc_base/numerics/safe_minmax.h>

namespace xrtc {
    namespace {

        const double kDefaultTrendlineSmoothingCoef = 0.9;
        const size_t kDefaultTrendlineWindowSize = 20;
        const double kDefaultTrendlineThresholdGain = 4.0;
        const int kMinNumDeltas = 60;
        const double kOverUsingTimeThreshold = 10;
        const double kMaxAdaptOffset = 15.0;

    } // namespace

    xrtc::TrendlineEstimator::TrendlineEstimator() :
            smoothing_coef_(kDefaultTrendlineSmoothingCoef),
            threshold_gain_(kDefaultTrendlineThresholdGain)
    {
        /* x_time_.open("D:/test/x_time.txt");
         y_trend_.open("D:/test/y_trend.txt");
         y_threshold_.open("D:/test/y_threshold.txt");*/
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

    webrtc::BandwidthUsage TrendlineEstimator::State() const {
        return hypothesis_;
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

        // 统计样本的个数
        ++num_of_deltas_;

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

        if (delay_hist_.size() > kDefaultTrendlineWindowSize) {
            delay_hist_.pop_front();
        }

        // 当样本数据满足要求，计算trend值
        double trend = prev_trend_;
        if (delay_hist_.size() == kDefaultTrendlineWindowSize) {
            trend = LinearFitSlope(delay_hist_).value_or(trend);
        }

        // 根据trend值进行过载检测
        Detect(trend, send_delta_ms, arrival_time_ms);
    }

// 线性回归最小二乘法
    absl::optional<double> TrendlineEstimator::LinearFitSlope(
            const std::deque<PacketTiming>& packets)
    {
        double sum_x = 0.0f;
        double sum_y = 0.0f;
        for (const auto& packet : packets) {
            sum_x += packet.arrival_time_ms;
            sum_y += packet.smoothed_delay_ms;
        }

        // 计算x，y的平均值
        double avg_x = sum_x / packets.size();
        double avg_y = sum_y / packets.size();

        // 分别计算分子和分母
        double num = 0.0f;
        double den = 0.0f;
        for (const auto& packet : packets) {
            double x = packet.arrival_time_ms;
            double y = packet.smoothed_delay_ms;

            num += (x - avg_x) * (y - avg_y);
            den += (x - avg_x) * (x - avg_x);
        }

        if (0.0f == den) {
            return absl::nullopt;
        }

        return num / den;
    }

    void TrendlineEstimator::Detect(double trend, double ts_delta,
                                    int64_t now_ms)
    {
        if (num_of_deltas_ < 2) {
            hypothesis_ = webrtc::BandwidthUsage::kBwNormal;
            return;
        }

        // 1. 对原始的trend值进行增益处理，增加区分度
        double modified_trend =
                std::min(num_of_deltas_, kMinNumDeltas) * trend * threshold_gain_;

        // 2. 进行过载检测
        if (modified_trend > threshold_) { // 有可能过载了
            if (-1 == time_over_using_) { // 第一次超过阈值
                time_over_using_ = ts_delta / 2;
            }
            else {
                time_over_using_ += ts_delta;
            }

            ++overuse_counter_;

            if (time_over_using_ > kOverUsingTimeThreshold && overuse_counter_ > 1) {
                if (trend > prev_trend_) {
                    // 判定过载
                    time_over_using_ = 0;
                    overuse_counter_ = 0;
                    hypothesis_ = webrtc::BandwidthUsage::kBwOverusing;
                }
            }
        }
        else if (modified_trend < -threshold_) {
            // 判定负载过低了
            time_over_using_ = -1;
            overuse_counter_ = 0;
            hypothesis_ = webrtc::BandwidthUsage::kBwUnderusing;
        }
        else {
            // 判定正常网络状态
            time_over_using_ = -1;
            overuse_counter_ = 0;
            hypothesis_ = webrtc::BandwidthUsage::kBwNormal;
        }

        prev_trend_ = trend;

        // 阈值的动态自适应调整
        UpdateThreshold(modified_trend, now_ms);

        // for test
        /* x_time_ << now_ms - first_arrival_time_ms_ << std::endl;
         y_trend_ << modified_trend << std::endl;
         y_threshold_ << threshold_ << std::endl;*/
    }

    void TrendlineEstimator::UpdateThreshold(double modified_trend,
                                             int64_t now_ms)
    {
        if (-1 == last_update_ms_) {
            last_update_ms_ = now_ms;
        }

        // 如果modified_trend异常大的时候，忽略本次更新
        if (modified_trend > threshold_ + kMaxAdaptOffset) {
            last_update_ms_ = now_ms;
            return;
        }

        // 调整阈值，当阈值调小的时候，使用的系数0.039
        // 当阈值调大的时候，使用的系数是0.0087
        double k = fabs(modified_trend) < threshold_ ? k_down_ : k_up_;
        const int64_t kMaxTimeDelta = 100;
        int64_t time_delta_ms = std::min(now_ms - last_update_ms_, kMaxTimeDelta);
        threshold_ += k * (fabs(modified_trend) - threshold_) * time_delta_ms;
        threshold_ = rtc::SafeClamp(threshold_, 6.0f, 600.0f);
        last_update_ms_ = now_ms;
    }

} // namespace xrtc
