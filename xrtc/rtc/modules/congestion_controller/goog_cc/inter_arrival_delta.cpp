#include "xrtc/rtc/modules/congestion_controller/goog_cc/inter_arrival_delta.h"

#include <rtc_base/logging.h>

namespace xrtc {
    namespace {

        constexpr webrtc::TimeDelta kBurstDeltaThreshold = webrtc::TimeDelta::Millis(5);
        constexpr webrtc::TimeDelta kMaxBurstDuration = webrtc::TimeDelta::Millis(100);
        constexpr webrtc::TimeDelta kArrivalTimeOffsetThreshold = webrtc::TimeDelta::Seconds(3);

    } // namespace

    InterArrivalDelta::InterArrivalDelta(
            webrtc::TimeDelta send_time_group_length) :
            send_time_group_length_(send_time_group_length)
    {
    }

    InterArrivalDelta::~InterArrivalDelta() {
    }

    bool InterArrivalDelta::ComputeDeltas(webrtc::Timestamp send_time,
                                          webrtc::Timestamp arrival_time,
                                          webrtc::Timestamp system_time,
                                          size_t packet_size,
                                          webrtc::TimeDelta* send_time_delta,
                                          webrtc::TimeDelta* arrival_time_delta,
                                          int* packet_size_delta)
    {
        bool calculated_delta = false;
        // 如果是第一个包
        if (current_timestamp_group_.IsFirstPacket()) {
            current_timestamp_group_.send_time = send_time;
            current_timestamp_group_.first_send_time = send_time;
            current_timestamp_group_.first_arrival = arrival_time;
        }
        else if (send_time < current_timestamp_group_.send_time) {
            return false;
        }
            // 是否需要创建新的分组
        else if (NewTimestampGroup(arrival_time, send_time)) {
            // 判断是否需要计算两个包组之间的时间差
            if (prev_timestamp_group_.complete_time.IsFinite()) {
                *send_time_delta = current_timestamp_group_.send_time -
                                   prev_timestamp_group_.send_time;
                *arrival_time_delta = current_timestamp_group_.complete_time -
                                      prev_timestamp_group_.complete_time;
                webrtc::TimeDelta system_time_delta = current_timestamp_group_.last_system_time -
                                                      prev_timestamp_group_.last_system_time;
                if (*arrival_time_delta - system_time_delta >=
                    kArrivalTimeOffsetThreshold)
                {
                    RTC_LOG(LS_WARNING) << "Arrival time clock offset has changed, "
                                        << "diff: " << arrival_time_delta->ms() - system_time_delta.ms()
                                        << ", resetting";
                    Reset();
                    return false;
                }
            }

            *packet_size_delta = static_cast<int>(current_timestamp_group_.size -
                                                  prev_timestamp_group_.size);
            calculated_delta = true;

            prev_timestamp_group_ = current_timestamp_group_;
            current_timestamp_group_.first_send_time = send_time;
            current_timestamp_group_.send_time = send_time;
            current_timestamp_group_.first_arrival = arrival_time;
            current_timestamp_group_.size = 0;
        }
        else { // 当前分组的包
            current_timestamp_group_.send_time =
                    std::max(current_timestamp_group_.send_time, send_time);
        }

        // 累计分组的包大小
        current_timestamp_group_.size += packet_size;
        current_timestamp_group_.complete_time = arrival_time;
        current_timestamp_group_.last_system_time = system_time;

        return calculated_delta;
    }

    bool InterArrivalDelta::NewTimestampGroup(webrtc::Timestamp arrival_time,
                                              webrtc::Timestamp send_time) const
    {
        if (current_timestamp_group_.complete_time.IsInfinite()) {
            return false;
        }
            // 突发的包
        else if (BelongsToBurst(arrival_time, send_time)) {
            return false;
        }
        else {
            return send_time - current_timestamp_group_.first_send_time >
                   send_time_group_length_;
        }
    }

    bool InterArrivalDelta::BelongsToBurst(webrtc::Timestamp arrival_time,
                                           webrtc::Timestamp send_time) const
    {
        webrtc::TimeDelta arrival_time_delta =
                arrival_time - current_timestamp_group_.complete_time;
        webrtc::TimeDelta send_time_delta =
                send_time - current_timestamp_group_.send_time;
        // 计算传播延迟差
        webrtc::TimeDelta propgation_delta = arrival_time_delta -
                                             send_time_delta;

        if (send_time_delta.IsZero()) { // 发送时间相同的包
            return true;
        }
        else if (propgation_delta < webrtc::TimeDelta::Zero() &&
                 arrival_time_delta < kBurstDeltaThreshold &&
                 arrival_time - current_timestamp_group_.first_arrival
                 < kMaxBurstDuration)
        {
            return true;
        }

        return false;
    }

    void InterArrivalDelta::Reset() {
        current_timestamp_group_ = SendTimeGroup();
        prev_timestamp_group_ = SendTimeGroup();
    }

} // namespace xrtc