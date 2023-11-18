//
// Created by faker on 2023/11/17.
//

#ifndef XRTCSDK_AIMD_RATE_CONTROL_H
#define XRTCSDK_AIMD_RATE_CONTROL_H


#include <api/units/data_rate.h>
#include <api/units/timestamp.h>

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

    private:
        webrtc::DataRate min_config_bitrate_;
        webrtc::DataRate max_config_bitrate_;
        webrtc::DataRate current_bitrate_;
        bool bitrate_is_init_ = false;
    };

} // namespace xrtc
#endif //XRTCSDK_AIMD_RATE_CONTROL_H
