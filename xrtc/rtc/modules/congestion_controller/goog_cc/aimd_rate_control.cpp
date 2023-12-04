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

    webrtc::DataRate AimdRateControl::ClampBitrate(webrtc::DataRate new_bitrate) {
        new_bitrate = std::max(new_bitrate, min_config_bitrate_);
        return new_bitrate;
    }

} // namespace xrtc