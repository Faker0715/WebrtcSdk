//
// Created by faker on 2023/6/24.
//

#ifndef XRTCSDK_RTCP_SENDER_H
#define XRTCSDK_RTCP_SENDER_H

#include <functional>
#include <set>
#include <map>

#include <api/units/time_delta.h>
#include <api/rtp_headers.h>
#include <rtc_base/random.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_interface.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"

namespace xrtc {

    class RTCPSender {
    public:
        class FeedbackState {
        public:
            uint32_t packets_sent;
            size_t media_bytes_sent;
        };

        RTCPSender(const RtpRtcpInterface::Configuration& config,
                   std::function<void(webrtc::TimeDelta)> schedule_next_rtcp_send);
        ~RTCPSender();

        void SetRTCPStatus(webrtc::RtcpMode mode);
        void SetSendingStatus(bool sending) {
            sending_ = sending;
        }

        void SetLastRtpTimestamp(uint32_t rtp_timestamp,
                                 absl::optional<webrtc::Timestamp> last_frame_capture_time);

        bool TimeToSendRTCPPacket(bool send_before_keyframe = false);
        int SendRTCP(const FeedbackState& feedback_state,
                     RTCPPacketType packet_type,
                     size_t nack_size = 0,
                     const uint16_t* nack_list = 0);

    private:
        class RtcpContext;
        class PacketSender;

        struct ReportFlag {
            ReportFlag(uint32_t type, bool is_volatile) :
                    type(type), is_volatile(is_volatile) {}
            bool operator<(const ReportFlag& flag) const {
                return type < flag.type;
            }

            bool operator==(const ReportFlag& flag) const {
                return type == flag.type;
            }

            uint32_t type;
            bool is_volatile;
        };

        void SetNextRtcpSendEvaluationDuration(webrtc::TimeDelta duration);
        int ComputeCompoundRTCPPacket(const FeedbackState& feedback_state,
                                      RTCPPacketType packet_type,
                                      size_t nack_size,
                                      const uint16_t* nack_list,
                                      PacketSender& sender);
        void SetFlag(uint32_t type);
        bool ConsumeFlag(uint32_t type, bool forced = false);
        bool AllVolatileFlagsConsumed();
        void PrepareReport(const FeedbackState& feedback_state);

        void BuildSr(const RtcpContext& context, PacketSender& sender);

    private:
        bool audio_;
        webrtc::Clock* clock_;
        uint32_t ssrc_;
        int clock_rate_;
        size_t max_packet_size_;
        RtpRtcpModuleObserver* rtp_rtcp_module_observer_;
        std::function<void(webrtc::TimeDelta)> schedule_next_rtcp_send_;
        webrtc::RtcpMode mode_ = webrtc::RtcpMode::kOff;
        webrtc::TimeDelta report_interval_ms_;
        std::set<ReportFlag> report_flags_;
        bool sending_ = false;
        webrtc::Random random_;
        typedef void (RTCPSender::* BuilderFunc)(const RtcpContext& context,
                                                 PacketSender& sender);
        std::map<uint32_t, BuilderFunc> builders_;
        // 最后一个rtp的时间戳
        uint32_t last_rtp_timestamp_ = 0;
        // 最后一个音频或者视频的采集时间
        absl::optional<webrtc::Timestamp> last_frame_capture_time_;
        absl::optional<webrtc::Timestamp> next_time_to_send_rtcp_;
    };

} // namespace xrtc



#endif //XRTCSDK_RTCP_SENDER_H
