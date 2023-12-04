#include "xrtc/rtc/pc/rtp_transport_controller_send.h"

#include <rtc_base/logging.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"
#include "xrtc/rtc/modules/congestion_controller/goog_cc/goog_cc_network_controller.h"

namespace xrtc {

    RtpTransportControllerSend::RtpTransportControllerSend(webrtc::Clock* clock,
                                                           PacingController::PacketSender* packet_sender,
                                                           webrtc::TaskQueueFactory* task_queue_factory) :
            clock_(clock),
            task_queue_pacer_(std::make_unique<TaskQueuePacedSender>(clock,
                                                                     packet_sender, task_queue_factory,
                                                                     webrtc::TimeDelta::Millis(1))),
            task_queue_(task_queue_factory->CreateTaskQueue("rtp_send_task_queue",
                                                            webrtc::TaskQueueFactory::Priority::NORMAL))
    {
        task_queue_pacer_->EnsureStarted();
    }

    RtpTransportControllerSend::~RtpTransportControllerSend() {
    }

    void RtpTransportControllerSend::EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet) {
        task_queue_pacer_->EnqueuePacket(std::move(packet));
    }

    void RtpTransportControllerSend::OnNetworkOk(bool network_ok) {
        RTC_LOG(LS_INFO) << "network state, is network ok: " << network_ok;
        webrtc::NetworkAvailability msg;
        msg.at_time = webrtc::Timestamp::Millis(clock_->TimeInMilliseconds());
        msg.network_available = network_ok;

        task_queue_.PostTask([this, msg]() {
            if (network_ok_ == msg.network_available) {
                return;
            }

            network_ok_ = msg.network_available;

            if (controller_) {

            }
            else {
                MaybeCreateController();
            }
        });
    }

    void RtpTransportControllerSend::OnSentPacket(
            const rtc::SentPacket& sent_packet)
    {
        task_queue_.PostTask([this, sent_packet]() {
            transport_feedback_adapter_.ProcessSentPacket(sent_packet);
        });
    }

    void RtpTransportControllerSend::OnRttUpdate(int64_t rtt_ms) {
        task_queue_.PostTask([this, rtt_ms]() {
            controller_->OnRttUpdate(rtt_ms);
        });
    }

    void RtpTransportControllerSend::OnAddPacket(
            const RtpPacketSendInfo& send_info)
    {
        webrtc::Timestamp creation_time = webrtc::Timestamp::Millis(
                clock_->TimeInMilliseconds());
        task_queue_.PostTask([this, creation_time, send_info]() {
            transport_feedback_adapter_.AddPacket(creation_time, 0,
                                                  send_info);
        });
    }

    void RtpTransportControllerSend::OnTransportFeedback(
            const rtcp::TransportFeedback& feedback)
    {
        webrtc::Timestamp feedback_time =
                webrtc::Timestamp::Millis(clock_->TimeInMilliseconds());
        task_queue_.PostTask([this, feedback, feedback_time]() {
            absl::optional<webrtc::TransportPacketsFeedback> feedback_msg =
                    transport_feedback_adapter_.ProcessTransportFeedback(
                            feedback, feedback_time);
            if (feedback_msg && controller_) {
                controller_->OnTransportPacketsFeedback(*feedback_msg);
            }
        });
    }

    void RtpTransportControllerSend::MaybeCreateController() {
        if (!network_ok_) {
            return;
        }

        controller_ = std::make_unique<GoogCcNetworkController>();
    }

} // namespace xrtc