//
// Created by faker on 2023/6/30.
//

#ifndef XRTCSDK_RTP_TRANSPORT_CONTROLLER_SEND_H
#define XRTCSDK_RTP_TRANSPORT_CONTROLLER_SEND_H

#include <system_wrappers/include/clock.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"
#include "xrtc/rtc/modules/pacing/task_queue_paced_sender.h"
#include "xrtc/rtc/modules/congestion_controller/rtp/transport_feedback_adapter.h"
#include "xrtc/rtc/pc/network_controller.h"

namespace xrtc {

    class RtpTransportControllerSend : public TransportFeedbackObserver {
    public:
        RtpTransportControllerSend(webrtc::Clock* clock,
                                   PacingController::PacketSender* packet_sender,
                                   webrtc::TaskQueueFactory* task_queue_factory);
        ~RtpTransportControllerSend();

        void EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet);

        void OnNetworkOk(bool network_ok);

        // TransportFeedbackObserver
        void OnTransportFeedback(
                const rtcp::TransportFeedback& feedback) override;

    private:
        void MaybeCreateController();

    private:
        webrtc::Clock* clock_;
        std::unique_ptr<TaskQueuePacedSender> task_queue_pacer_;
        std::unique_ptr<NetworkControllerInterface> controller_;
        bool network_ok_ = false;
        rtc::TaskQueue task_queue_;
        TransportFeedbackAdapter transport_feedback_adapter_;
    };

} // namespace xrtc



#endif //XRTCSDK_RTP_TRANSPORT_CONTROLLER_SEND_H
