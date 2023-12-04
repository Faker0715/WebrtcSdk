#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_

#include <api/transport/network_types.h>
#include <api/units/data_rate.h>
#include <absl/types/optional.h>

#include "xrtc/rtc/modules/congestion_controller/goog_cc/inter_arrival_delta.h"
#include "xrtc/rtc/modules/congestion_controller/goog_cc/trendline_estimator.h"
#include "xrtc/rtc/modules/congestion_controller/goog_cc/aimd_rate_control.h"

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
                const webrtc::TransportPacketsFeedback& msg,
                absl::optional<webrtc::DataRate> acked_bitrate);
        void OnRttUpdate(int64_t rtt_ms);

    private:
        void IncomingPacketFeedback(const webrtc::PacketResult& packet_feedback,
                                    webrtc::Timestamp at_time);
        Result MaybeUpdateEstimate(
                absl::optional<webrtc::DataRate> acked_bitrate,
                bool recover_from_overusing,
                webrtc::Timestamp at_time);

    private:
        std::unique_ptr<InterArrivalDelta> video_inter_arrival_delta_;
        std::unique_ptr<TrendlineEstimator> video_delay_detector_;
        webrtc::Timestamp last_seen_timestamp_ = webrtc::Timestamp::MinusInfinity();
        AimdRateControl rate_control_;
    };

} // namespace xrtc


#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_BASED_BWE_H_