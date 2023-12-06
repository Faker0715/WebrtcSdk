#include "xrtc/rtc/modules/congestion_controller/goog_cc/aimd_rate_control.h"

#include <rtc_base/logging.h>

namespace xrtc {
    namespace {

        constexpr webrtc::TimeDelta kDefaultRtt = webrtc::TimeDelta::Millis(200);

    } // namespace

    AimdRateControl::AimdRateControl() :
            min_config_bitrate_(webrtc::DataRate::KilobitsPerSec(5)),
            max_config_bitrate_(webrtc::DataRate::KilobitsPerSec(30000)),
            current_bitrate_(max_config_bitrate_),
            latest_estimated_throughput_(current_bitrate_),
            rtt_(kDefaultRtt)
    {
    }

    AimdRateControl::~AimdRateControl() {
    }

    void AimdRateControl::SetStartBitrate(webrtc::DataRate start_bitrate) {
        current_bitrate_ = start_bitrate;
        bitrate_is_init_ = true;
    }

    bool AimdRateControl::ValidEstimate() const {
        return bitrate_is_init_;
    }

    bool AimdRateControl::TimeToReduceFurther(webrtc::Timestamp at_time,
                                              webrtc::DataRate estimated_throughput) const
    {
        // 为了防止码率降低的过于频繁，需要控制码率降低的频率
        // 两次码率降低的间隔，要大于1个RTT
        webrtc::TimeDelta bitrate_reduction_interval =
                rtt_.Clamped(webrtc::TimeDelta::Millis(10),
                             webrtc::TimeDelta::Millis(200));
        if (at_time - time_last_bitrate_change_ >= rtt_) {
            return true;
        }

        // 当前码率的一半必须要大于吞吐量，避免码率降得过低
        if (ValidEstimate()) {
            webrtc::DataRate threshold = LatestEstimate() * 0.5;
            return estimated_throughput < threshold;
        }

        return false;
    }

    bool AimdRateControl::InitialTimeToReduceFurther(
            webrtc::Timestamp at_time) const
    {
        return ValidEstimate() && TimeToReduceFurther(at_time,
                                                      LatestEstimate() / 2 - webrtc::DataRate::BitsPerSec(1));
    }

    webrtc::DataRate AimdRateControl::LatestEstimate() const {
        return current_bitrate_;
    }

    void AimdRateControl::SetRtt(webrtc::TimeDelta rtt) {
        rtt_ = rtt;
        RTC_LOG(LS_WARNING) << "==========rtt: " << rtt.ms();
    }

    void AimdRateControl::SetEstimate(webrtc::DataRate new_bitrate,
                                      webrtc::Timestamp at_time)
    {
        bitrate_is_init_ = true;
        webrtc::DataRate prev_bitrate = current_bitrate_;
        current_bitrate_ = ClampBitrate(new_bitrate);
        time_last_bitrate_change_ = at_time;
        if (current_bitrate_ < prev_bitrate) {
            time_last_bitrate_decrease_ = at_time;
        }
    }

    webrtc::DataRate AimdRateControl::Update(
            absl::optional<webrtc::DataRate> throughput_estimate,
            webrtc::BandwidthUsage state,
            webrtc::Timestamp at_time)
    {
        if (!bitrate_is_init_) {
            const webrtc::TimeDelta kInitTime = webrtc::TimeDelta::Seconds(5);
            if (time_first_throughput_estimate_.IsInfinite()) {
                time_first_throughput_estimate_ = at_time;
            }
            else if (at_time - time_first_throughput_estimate_ > kInitTime) {
                current_bitrate_ = *throughput_estimate;
                bitrate_is_init_ = true;
            }
        }

        ChangeBitrate(throughput_estimate, state, at_time);

        return current_bitrate_;
    }

    webrtc::DataRate AimdRateControl::ClampBitrate(webrtc::DataRate new_bitrate) {
        new_bitrate = std::max(new_bitrate, min_config_bitrate_);
        return new_bitrate;
    }

    void AimdRateControl::ChangeBitrate(
            absl::optional<webrtc::DataRate> acked_bitrate,
            webrtc::BandwidthUsage state,
            webrtc::Timestamp at_time)
    {
        webrtc::DataRate estimated_throughput =
                acked_bitrate.value_or(latest_estimated_throughput_);
        // 更新最新的吞吐量
        if (acked_bitrate) {
            latest_estimated_throughput_ = *acked_bitrate;
        }

        // 当前网络状态是过载状态，即使没有初始化起始码率，仍然需要降低码率
        if (!bitrate_is_init_ && state == webrtc::BandwidthUsage::kBwOverusing) {
            return;
        }

        ChangeState(state, at_time);
    }

    void AimdRateControl::ChangeState(webrtc::BandwidthUsage state,
                                      webrtc::Timestamp at_time)
    {
        switch (state) {
            case webrtc::BandwidthUsage::kBwNormal:
                if (rate_control_state_ == RateControlState::kRcHold) {
                    rate_control_state_ = RateControlState::kRcIncrease;
                    time_last_bitrate_change_ = at_time;
                }
                break;
            case webrtc::BandwidthUsage::kBwOverusing:
                if (rate_control_state_ != RateControlState::kRcDecrease) {
                    rate_control_state_ = RateControlState::kRcDecrease;
                }
                break;
            case webrtc::BandwidthUsage::kBwUnderusing:
                rate_control_state_ = RateControlState::kRcHold;
                break;
            default:
                break;
        }
    }

} // namespace xrtc