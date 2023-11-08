//
// Created by faker on 2023/6/27.
//

#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTCP_RECEIVER_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTCP_RECEIVER_H_

#include <vector>

#include <api/array_view.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/common_header.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/report_block.h"

namespace xrtc {

    class RTCPReceiver {
    public:
        RTCPReceiver(const RtpRtcpInterface::Configuration& config);
        ~RTCPReceiver();

        void IncomingRtcpPacket(rtc::ArrayView<const uint8_t> packet);

    private:
        class PacketInformation;

        bool ParseCompoundPacket(rtc::ArrayView<const uint8_t> packet,
                                 PacketInformation& packet_info);
        void HandleReceiverReport(const rtcp::CommonHeader& rtcp_block,
                                  PacketInformation& packet_info);
        void HandleReportBlock(const rtcp::ReportBlock& report_block,
                               uint32_t remote_ssrc, PacketInformation& packet_info);
        void HandleNack(const rtcp::CommonHeader& rtcp_block,
                        PacketInformation& packet_info);
        void HandleTransportFeedback(const rtcp::CommonHeader& rtcp_block,
                                     PacketInformation& packet_info);

        bool IsRegisteredSsrc(uint32_t ssrc);

    private:
        webrtc::Clock* clock_;
        bool audio_;
        RtpRtcpModuleObserver* rtp_rtcp_module_observer_;
        uint32_t num_skipped_packets_ = 0;
        std::vector<uint32_t> registered_ssrcs_;
        webrtc::Timestamp last_received_rb_ = webrtc::Timestamp::PlusInfinity();
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTCP_RECEIVER_H_