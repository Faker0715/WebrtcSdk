//
// Created by faker on 2023/6/27.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_receiver.h"

#include <rtc_base/logging.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/receiver_report.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/nack.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/transport_feedback.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_utils.h"

namespace xrtc {

    struct RTCPReceiver::PacketInformation {

    };

    RTCPReceiver::RTCPReceiver(const RtpRtcpInterface::Configuration& config) :
            clock_(config.clock),
            audio_(config.audio),
            rtp_rtcp_module_observer_(config.rtp_rtcp_module_observer)
    {
        registered_ssrcs_.push_back(config.local_media_ssrc);
    }

    RTCPReceiver::~RTCPReceiver() {
    }

    void RTCPReceiver::IncomingRtcpPacket(rtc::ArrayView<const uint8_t> packet) {
        if (packet.empty()) {
            RTC_LOG(LS_WARNING) << "rtcp packet is empty";
            return;
        }

        PacketInformation packet_info;
        if (!ParseCompoundPacket(packet, packet_info)) {
            return;
        }
    }

    bool RTCPReceiver::ParseCompoundPacket(rtc::ArrayView<const uint8_t> packet,
                                           PacketInformation& packet_info)
    {
        rtcp::CommonHeader rtcp_block;
        for (const uint8_t* next_block = packet.begin();
             next_block != packet.end();
             next_block = rtcp_block.NextPacket())
        {
            ptrdiff_t remaining_packet_size = packet.end() - next_block;
            if (!rtcp_block.Parse(next_block, remaining_packet_size)) {
                if (next_block == packet.begin()) {
                    RTC_LOG(LS_WARNING) << "parse rtcp packet failed";
                    return false;
                }

                ++num_skipped_packets_;
                break;
            }

            switch (rtcp_block.packet_type()) {
                case rtcp::ReceiverReport::kPacketType: // 201
                    HandleReceiverReport(rtcp_block, packet_info);
                    break;
                case rtcp::Rtpfb::kPacketType: // 205
                    switch (rtcp_block.fmt()) {
                        case rtcp::Nack::kFeedbackMessageType: // 1
                            HandleNack(rtcp_block, packet_info);
                            break;
                        case rtcp::TransportFeedback::kFeedbackMessageType: // 15
                            //HandleTransportFeedback(rtcp_block, packet_info);
                            RTC_LOG(LS_WARNING) << "============transport feedback: "
                                                << static_cast<int>(rtcp_block.fmt());
                            break;
                        default:
                            ++num_skipped_packets_;
                            break;
                    }
                    break;
                default:
                    RTC_LOG(LS_WARNING) << "rtcp packet not handle, packet_type: " <<
                                        (int)(rtcp_block.packet_type());
                    break;
            }
        }

        return false;
    }

    void RTCPReceiver::HandleReceiverReport(const rtcp::CommonHeader& rtcp_block,
                                            PacketInformation& packet_info)
    {
        rtcp::ReceiverReport rr;
        if (!rr.Parse(rtcp_block)) {
            ++num_skipped_packets_;
            return;
        }

        uint32_t remote_ssrc = rr.sender_ssrc();

        for (const rtcp::ReportBlock& report_block : rr.report_blocks()) {
            HandleReportBlock(report_block, remote_ssrc, packet_info);
        }
    }

    void RTCPReceiver::HandleReportBlock(const rtcp::ReportBlock& report_block,
                                         uint32_t remote_ssrc,
                                         PacketInformation& packet_info)
    {
        if (!IsRegisteredSsrc(report_block.source_ssrc())) {
            return;
        }

        last_received_rb_ = clock_->CurrentTime();

        // 计算RTT的值
        int64_t rtt_ms = 0;
        uint32_t send_ntp_time = report_block.last_sr();
        if (send_ntp_time != 0) {
            uint32_t delay_ntp = report_block.delay_since_last_sr();
            uint32_t receive_ntp_time =
                    CompactNtp(clock_->ConvertTimestampToNtpTime(last_received_rb_));
            uint32_t rtt_ntp = receive_ntp_time - send_ntp_time - delay_ntp;
            // 需要将rtt_ntp转换成ms
            rtt_ms = CompactNtpRttToMs(rtt_ntp);

            if (rtp_rtcp_module_observer_) {
                rtp_rtcp_module_observer_->OnNetworkInfo(rtt_ms,
                                                         report_block.packets_lost(),
                                                         report_block.fraction_lost(),
                                                         report_block.jitter());
            }
        }
    }

    void RTCPReceiver::HandleNack(const rtcp::CommonHeader& rtcp_block,
                                  PacketInformation& packet_info)
    {
        rtcp::Nack nack;
        if (!nack.Parse(rtcp_block)) {
            ++num_skipped_packets_;
            return;
        }

        if (rtp_rtcp_module_observer_) {
            rtp_rtcp_module_observer_->OnNackReceived(
                    audio_ ? webrtc::MediaType::AUDIO : webrtc::MediaType::VIDEO,
                    nack.packet_ids());
        }
    }

    bool RTCPReceiver::IsRegisteredSsrc(uint32_t ssrc) {
        for (auto rssrc : registered_ssrcs_) {
            if (rssrc == ssrc) {
                return true;
            }
        }
        return false;
    }

} // namespace xrtc

