//
// Created by faker on 2023/7/1.
//

#ifndef XRTCSDK_PACING_CONTROLLER_H
#define XRTCSDK_PACING_CONTROLLER_H


#include <system_wrappers/include/clock.h>
#include <api/units/data_rate.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"
#include "xrtc/rtc/modules/pacing/round_robin_packet_queue.h"
#include "xrtc/rtc/modules/pacing/interval_budget.h"

namespace xrtc {

    class PacingController {
    public:
        class PacketSender {
        public:
            virtual ~PacketSender() = default;
            virtual void SendPacket(std::unique_ptr<RtpPacketToSend> packet) = 0;
        };

        PacingController(webrtc::Clock* clock,
                         PacketSender* packet_sender);
        ~PacingController();

        void EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet);
        void ProcessPackets();
        webrtc::Timestamp NextSendTime();
        void SetPacingBitrate(webrtc::DataRate bitrate);
        void SetQueueTimeLimit(webrtc::TimeDelta limit) {
            queue_time_limit_ = limit;
        }

    private:
        void EnqueuePacketInternal(int priority,
                                   std::unique_ptr<RtpPacketToSend> packet);
        webrtc::TimeDelta UpdateTimeAndGetElapsed(webrtc::Timestamp now);
        void UpdateBudgetWithElapsedTime(webrtc::TimeDelta elapsed_time);
        void UpdateBudgetWithSendData(webrtc::DataSize size);
        std::unique_ptr<RtpPacketToSend> GetPendingPacket();
        void OnPacketSent(webrtc::DataSize packet_size, webrtc::Timestamp send_time);

    private:
        webrtc::Clock* clock_;
        PacketSender* packet_sender_;
        uint64_t packet_counter_ = 0;
        webrtc::Timestamp last_process_time_;
        RoundRobinPacketQueue packet_queue_;
        webrtc::TimeDelta min_packet_limit_;
        IntervalBudget media_budget_;
        webrtc::DataRate pacing_bitrate_;
        bool drain_large_queue_ = true;
        // 期望的最大延迟时间
        webrtc::TimeDelta queue_time_limit_;
    };

} // namespace xrtc

#endif //XRTCSDK_PACING_CONTROLLER_H
