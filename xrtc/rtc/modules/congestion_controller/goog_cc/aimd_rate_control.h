//
// Created by faker on 2023/11/17.
//

#ifndef XRTCSDK_AIMD_RATE_CONTROL_H
#define XRTCSDK_AIMD_RATE_CONTROL_H


#include <api/units/data_rate.h>
#include <api/units/timestamp.h>
#include <api/network_state_predictor.h>
#include <absl/types/optional.h>
namespace xrtc {

    class AimdRateControl {
    public:
        AimdRateControl();
        ~AimdRateControl();

        void SetStartBitrate(webrtc::DataRate start_bitrate);
        bool ValidEstimate() const;

        bool TimeToReduceFurther(webrtc::Timestamp at_time,
                                 webrtc::DataRate estimated_throughput) const;
        bool InitialTimeToReduceFurther(webrtc::Timestamp at_time) const;
        webrtc::DataRate LatestEstimate() const;
        void SetRtt(webrtc::TimeDelta rtt);
        void SetEstimate(webrtc::DataRate new_bitrate, webrtc::Timestamp at_time);
        webrtc::DataRate Update(absl::optional<webrtc::DataRate> throughput_estimate,
                                webrtc::BandwidthUsage state,
                                webrtc::Timestamp at_time);

    private:
        enum class RateControlState {
            kRcHold,     // 保持码率不变
            kRcIncrease, // 增加码率
            kRcDecrease, // 降低码率
        };

        webrtc::DataRate ClampBitrate(webrtc::DataRate new_bitrate);
        void ChangeBitrate(absl::optional<webrtc::DataRate> throughput_estimate,
                           webrtc::BandwidthUsage state,
                           webrtc::Timestamp at_time);
        void ChangeState(webrtc::BandwidthUsage state,
                         webrtc::Timestamp at_time);

    private:
        webrtc::DataRate min_config_bitrate_;
        webrtc::DataRate max_config_bitrate_;
        webrtc::DataRate current_bitrate_;
        webrtc::DataRate latest_estimated_throughput_;
        bool bitrate_is_init_ = false;
        webrtc::TimeDelta rtt_;
        webrtc::Timestamp time_last_bitrate_change_ = webrtc::Timestamp::MinusInfinity();
        webrtc::Timestamp time_last_bitrate_decrease_ = webrtc::Timestamp::MinusInfinity();
        webrtc::Timestamp time_first_throughput_estimate_ = webrtc::Timestamp::MinusInfinity();
        RateControlState rate_control_state_ = RateControlState::kRcHold;
    };

} // namespace xrtc
#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_AIMD_RATE_CONTROL_H_
