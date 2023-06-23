//
// Created by faker on 2023/6/23.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_impl.h"

#include <rtc_base/thread.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include <rtc_base/time_utils.h>

namespace xrtc {

    ModuleRtpRtcpImpl::ModuleRtpRtcpImpl(
            const RtpRtcpInterface::Configuration& config) :
            config_(config){
//            rtcp_sender_(config, [=](webrtc::TimeDelta duration) {
//                ScheduleNextRtcpSend(duration);
//            }),
//            rtcp_receiver_(config)
//    {
    }

    ModuleRtpRtcpImpl::~ModuleRtpRtcpImpl() {
    }

    void ModuleRtpRtcpImpl::UpdateRtpStats(std::shared_ptr<RtpPacketToSend> packet,
                                           bool is_rtx, bool is_retransmit)
    {
        StreamDataCounter* stream_counter = is_rtx ? &rtx_rtp_stats_ : &rtp_stats_;

        RtpPacketCounter counter(*packet);
        if (is_retransmit) {
            stream_counter->retransmitted.Add(counter);
        }

        stream_counter->transmmited.Add(counter);
    }

//    void ModuleRtpRtcpImpl::SetRTCPStatus(webrtc::RtcpMode mode) {
//        rtcp_sender_.SetRTCPStatus(mode);
//    }

//    void ModuleRtpRtcpImpl::SetSendingStatus(bool sending) {
//        rtcp_sender_.SetSendingStatus(sending);
//    }

//    void ModuleRtpRtcpImpl::OnSendingRtpFrame(uint32_t rtp_timestamp,
//                                              int64_t capture_time_ms,
//                                              bool forced_report)
//    {
//        absl::optional<webrtc::Timestamp> capture_time;
//        if (capture_time_ms > 0) {
//            capture_time = webrtc::Timestamp::Millis(capture_time_ms);
//        }
//
//        rtcp_sender_.SetLastRtpTimestamp(rtp_timestamp, capture_time);
//
//        if (rtcp_sender_.TimeToSendRTCPPacket(forced_report)) {
//            rtcp_sender_.SendRTCP(GetFeedbackState(), kRtcpReport);
//        }
//    }

//    void ModuleRtpRtcpImpl::IncomingRtcpPacket(rtc::ArrayView<const uint8_t> packet) {
//        rtcp_receiver_.IncomingRtcpPacket(packet);
//    }

//    void ModuleRtpRtcpImpl::ScheduleNextRtcpSend(webrtc::TimeDelta duration) {
//        if (duration.IsZero()) {
//            MaybeSendRTCP();
//        }
//        else {
//            webrtc::Timestamp execute_time = config_.clock->CurrentTime() + duration;
//            ScheduleMaybeSendRtcpAtOrAfterTimestamp(execute_time, duration);
//        }
//    }

//    void ModuleRtpRtcpImpl::MaybeSendRTCP() {
//        if (rtcp_sender_.TimeToSendRTCPPacket()) {
//            rtcp_sender_.SendRTCP(GetFeedbackState(), RTCPPacketType::kRtcpReport);
//        }
//    }

//    static int DelayMillisForDuration(webrtc::TimeDelta duration) {
//        return (duration.us() + rtc::kNumMillisecsPerSec - 1) / rtc::kNumMicrosecsPerMillisec;
//    }
//
//    void ModuleRtpRtcpImpl::ScheduleMaybeSendRtcpAtOrAfterTimestamp(
//            webrtc::Timestamp execute_time,
//            webrtc::TimeDelta duration)
//    {
//        rtc::Thread::Current()->PostDelayedTask(webrtc::ToQueuedTask(task_safety_, [=]() {
//            webrtc::Timestamp now = config_.clock->CurrentTime();
//            if (now >= execute_time) {
//                MaybeSendRTCP();
//                return;
//            }
//
//            ScheduleMaybeSendRtcpAtOrAfterTimestamp(execute_time, execute_time - now);
//
//        }), DelayMillisForDuration(duration));
//    }

//    RTCPSender::FeedbackState ModuleRtpRtcpImpl::GetFeedbackState() {
//        RTCPSender::FeedbackState feedback_state;
//        if (!config_.receiver_only) {
//            feedback_state.packets_sent = rtp_stats_.transmmited.packets +
//                                          rtx_rtp_stats_.transmmited.packets;
//            feedback_state.media_bytes_sent = rtp_stats_.transmmited.payload_bytes
//                                              + rtx_rtp_stats_.transmmited.payload_bytes;
//        }
//
//        return feedback_state;
//    }

} // namespace xrtc

