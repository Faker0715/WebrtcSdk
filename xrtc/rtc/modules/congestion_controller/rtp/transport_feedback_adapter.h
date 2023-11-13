#ifndef XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_
#define XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_
#include <map>

#include <modules/include/module_common_types_public.h>
#include <api/units/time_delta.h>
#include <api/transport/network_types.h>
#include <rtc_base/network/sent_packet.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"

namespace xrtc {

    struct PacketFeedback {
        PacketFeedback() = default;

        webrtc::Timestamp creation_time = webrtc::Timestamp::MinusInfinity();
        webrtc::SentPacket sent;
        webrtc::Timestamp receive_time = webrtc::Timestamp::PlusInfinity();
    };

    class TransportFeedbackAdapter {
    public:
        TransportFeedbackAdapter();
        ~TransportFeedbackAdapter();

        absl::optional<webrtc::SentPacket> ProcessSentPacket(
                const rtc::SentPacket& sent_packet);
        void AddPacket(webrtc::Timestamp creation_time,
                       size_t overhead_bytes,
                       const RtpPacketSendInfo& send_info);
        absl::optional<webrtc::TransportPacketsFeedback> ProcessTransportFeedback(
                const rtcp::TransportFeedback& feedback,
                webrtc::Timestamp feedback_time);

    private:
        std::vector<webrtc::PacketResult> ProcessTransportFeedbackInner(
                const rtcp::TransportFeedback& feedback,
                webrtc::Timestamp feedback_time);

    private:
        webrtc::Timestamp current_offset_ = webrtc::Timestamp::MinusInfinity();
        webrtc::TimeDelta last_timestamp_ = webrtc::TimeDelta::MinusInfinity();
        std::map<int64_t, PacketFeedback> history_;
        webrtc::SequenceNumberUnwrapper seq_num_unwrapper_;
        webrtc::Timestamp last_send_time_ = webrtc::Timestamp::MinusInfinity();
        int64_t last_ack_seq_num_ = -1;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_CONGESTION_CONTROLLER_RTP_TRANSPORT_FEEDBACK_ADAPTER_H_