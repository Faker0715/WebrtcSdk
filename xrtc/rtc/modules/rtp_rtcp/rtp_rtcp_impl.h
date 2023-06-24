//
// Created by faker on 2023/6/23.
//

#ifndef XRTCSDK_RTP_RTCP_IMPL_H
#define XRTCSDK_RTP_RTCP_IMPL_H


#include <rtc_base/task_utils/pending_task_safety_flag.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_sender.h"
//#include "xrtc/rtc/modules/rtp_rtcp/rtcp_receiver.h"

namespace xrtc {

    class ModuleRtpRtcpImpl {
    public:
        ModuleRtpRtcpImpl(const RtpRtcpInterface::Configuration& config);
        ~ModuleRtpRtcpImpl();

        /**
         * @brief 发送rtp包
         * @param packet
         * @param is_rtx 是不是重传包 避免对拥塞控制有干扰
         * @param is_retransmit 重传不一定是rtx 音频包就不是
         */
        void UpdateRtpStats(std::shared_ptr<RtpPacketToSend> packet,
                            bool is_rtx,
                            bool is_retransmit);
//        void SetRTCPStatus(webrtc::RtcpMode mode);
//        void SetSendingStatus(bool sending);
//        void OnSendingRtpFrame(uint32_t rtp_timestamp,
//                               int64_t capture_time_ms,
//                               bool forced_report);
//        void IncomingRtcpPacket(const uint8_t* packet, size_t length) {
//            IncomingRtcpPacket(rtc::MakeArrayView(packet, length));
//        }
//        void IncomingRtcpPacket(rtc::ArrayView<const uint8_t> packet);

    private:
        void ScheduleNextRtcpSend(webrtc::TimeDelta duration);
        void MaybeSendRTCP();
        void ScheduleMaybeSendRtcpAtOrAfterTimestamp(
                webrtc::Timestamp execute_time,
                webrtc::TimeDelta duration);
        RTCPSender::FeedbackState GetFeedbackState();

    private:
        RtpRtcpInterface::Configuration config_;
        StreamDataCounter rtp_stats_;
        StreamDataCounter rtx_rtp_stats_;

        RTCPSender rtcp_sender_;
//        RTCPReceiver rtcp_receiver_;

        webrtc::ScopedTaskSafety task_safety_;
    };

} // namespace xrtc

#endif //XRTCSDK_RTP_RTCP_IMPL_H
