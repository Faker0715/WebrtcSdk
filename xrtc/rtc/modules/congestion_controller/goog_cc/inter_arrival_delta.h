//
// Created by faker on 2023/11/14.
//

#ifndef XRTCSDK_INTER_ARRIVAL_DELTA_H
#define XRTCSDK_INTER_ARRIVAL_DELTA_H


#include <api/units/time_delta.h>
#include <api/units/timestamp.h>

namespace xrtc {

    class InterArrivalDelta {
    public:
        InterArrivalDelta(webrtc::TimeDelta send_time_group_length);
        ~InterArrivalDelta();

        bool ComputeDeltas(webrtc::Timestamp send_time,
                           webrtc::Timestamp arrival_time,
                           webrtc::Timestamp system_time,
                           size_t packet_size,
                           webrtc::TimeDelta* send_time_delta,
                           webrtc::TimeDelta* arrival_time_delta,
                           int* packet_size_delta);

    private:
        struct SendTimeGroup {
            SendTimeGroup() :
                    size(0),
                    first_send_time(webrtc::Timestamp::MinusInfinity()),
                    send_time(webrtc::Timestamp::MinusInfinity()),
                    first_arrival(webrtc::Timestamp::MinusInfinity()),
                    complete_time(webrtc::Timestamp::MinusInfinity()),
                    last_system_time(webrtc::Timestamp::MinusInfinity())
            {
            }

            bool IsFirstPacket() const {
                return complete_time.IsInfinite();
            }

            // 分组中所有包的累计字节数
            size_t size;
            // 分组第一个包的发送时间
            webrtc::Timestamp first_send_time;
            // 分组最后一个包的发送时间
            webrtc::Timestamp send_time;
            // 分组中第一个达到接收端的包的时间
            webrtc::Timestamp first_arrival;
            // 分组中最后一个达到接收端的包的时间
            webrtc::Timestamp complete_time;
            // 最新的系统时间
            webrtc::Timestamp last_system_time;
        };

        bool NewTimestampGroup(webrtc::Timestamp arrival_time,
                               webrtc::Timestamp send_time) const;
        bool BelongsToBurst(webrtc::Timestamp arrival_time,
                            webrtc::Timestamp send_time) const;
        void Reset();

    private:
        webrtc::TimeDelta send_time_group_length_;
        SendTimeGroup current_timestamp_group_;
        SendTimeGroup prev_timestamp_group_;
    };

} // namespace xrtc


#endif //XRTCSDK_INTER_ARRIVAL_DELTA_H
