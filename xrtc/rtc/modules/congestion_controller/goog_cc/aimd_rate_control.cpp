//
// Created by faker on 2023/11/17.
//

#include "xrtc/rtc/modules/congestion_controller/goog_cc/aimd_rate_control.h"

namespace xrtc {

    AimdRateControl::AimdRateControl() :
            min_config_bitrate_(webrtc::DataRate::KilobitsPerSec(5)),
            max_config_bitrate_(webrtc::DataRate::KilobitsPerSec(30000)),
            current_bitrate_(max_config_bitrate_)
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

} // namespace xrtc