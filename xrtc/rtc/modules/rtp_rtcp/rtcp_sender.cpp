//
// Created by faker on 2023/6/24.
//

#include "xrtc/rtc/modules/rtp_rtcp/rtcp_sender.h"

#include <rtc_base/logging.h>

//#include "xrtc/rtc/modules/rtp_rtcp/rtcp_packet/sender_report.h"

namespace xrtc {
    namespace {

        // 音视频默认间隔
        const int kDefaultAudioRtcpIntervalMs = 5000;
        const int kDefaultVideoRtcpIntervalMs = 1000;
        constexpr webrtc::TimeDelta RTCP_BEFORE_KEY_FRAME = webrtc::TimeDelta::Millis(100);

        webrtc::TimeDelta GetReportInterval(int report_interval_ms, int default_value) {
            if (report_interval_ms > 0) {
                return webrtc::TimeDelta::Millis(report_interval_ms);
            }
            return webrtc::TimeDelta::Millis(default_value);
        }

    } // namespace

    class RTCPSender::RtcpContext {
    public:
        RtcpContext(const FeedbackState& feedback_state,
                    webrtc::Timestamp now) :
                feedback_state(feedback_state),
                now(now) {}

        FeedbackState feedback_state;
        webrtc::Timestamp now;
    };

    class RTCPSender::PacketSender {
    public:
        PacketSender(rtcp::RtcpPacket::PacketReadyCallback callback,
                     size_t max_packet_size) :
                callback_(callback), max_packet_size_(max_packet_size)
        { }

        ~PacketSender() {}

        void Append(rtcp::RtcpPacket& packet) {
            packet.Create(buffer_, &index_, max_packet_size_, callback_);
        }

        void Send() {
            if (index_ > 0) {
                callback_(rtc::ArrayView<const uint8_t>(buffer_, index_));
                index_ = 0;
            }
        }

    private:
        rtcp::RtcpPacket::PacketReadyCallback callback_;
        size_t max_packet_size_;
        uint8_t buffer_[IP_PACKET_SIZE];
        size_t index_ = 0;
    };

    RTCPSender::RTCPSender(const RtpRtcpInterface::Configuration& config,
                           std::function<void(webrtc::TimeDelta)> schedule_next_rtcp_send) :
            audio_(config.audio),
            clock_(config.clock),
            ssrc_(config.local_media_ssrc),
            clock_rate_(config.clock_rate),
            max_packet_size_(IP_PACKET_SIZE - 28), // IPv4 + UDP,
            rtp_rtcp_module_observer_(config.rtp_rtcp_module_observer),
            schedule_next_rtcp_send_(schedule_next_rtcp_send),
            report_interval_ms_(GetReportInterval(config.rtcp_report_interval_ms,
                                                  config.audio ? kDefaultAudioRtcpIntervalMs : kDefaultVideoRtcpIntervalMs)),
            random_(config.clock->TimeInMilliseconds())
    {
        builders_[kRtcpSr] = &RTCPSender::BuildSr;
    }

    RTCPSender::~RTCPSender() {
    }

    void RTCPSender::SetRTCPStatus(webrtc::RtcpMode mode) {
        if (mode == webrtc::RtcpMode::kOff) {
            // 关闭RTCP功能是禁止的
            return;
        }
        else if (mode_ == webrtc::RtcpMode::kOff) {
            SetNextRtcpSendEvaluationDuration(report_interval_ms_ / 2);
        }

        mode_ = mode;
    }

    void RTCPSender::SetLastRtpTimestamp(uint32_t rtp_timestamp,
                                         absl::optional<webrtc::Timestamp> last_frame_capture_time)
    {
        last_rtp_timestamp_ = rtp_timestamp;
        if (last_frame_capture_time.has_value()) {
            last_frame_capture_time_ = last_frame_capture_time;
        }
        else {
            last_frame_capture_time_ = clock_->CurrentTime();
        }
    }

    bool RTCPSender::TimeToSendRTCPPacket(bool send_before_keyframe) {
        webrtc::Timestamp now = clock_->CurrentTime();

        if (!audio_ && send_before_keyframe) {
            now += RTCP_BEFORE_KEY_FRAME;
        }
        return now >= *next_time_to_send_rtcp_;
    }

    int RTCPSender::SendRTCP(const FeedbackState& feedback_state,
                             RTCPPacketType packet_type,
                             size_t nack_size,
                             const uint16_t* nack_list)
    {
        absl::optional<PacketSender> sender;
        auto callback = [&](rtc::ArrayView<const uint8_t> packet) {
            // 可以获得打包后的复合包
            if (rtp_rtcp_module_observer_) {
                rtp_rtcp_module_observer_->OnLocalRtcpPacket(
                        audio_ ? webrtc::MediaType::AUDIO : webrtc::MediaType::VIDEO,
                        packet.data(), packet.size());
            }
        };
        sender.emplace(callback, max_packet_size_);

        int ret = ComputeCompoundRTCPPacket(feedback_state,
                                            packet_type, nack_size, nack_list, *sender);

        // 触发回调
        sender->Send();
        return ret;
    }

    void RTCPSender::SetNextRtcpSendEvaluationDuration(webrtc::TimeDelta duration) {
        next_time_to_send_rtcp_ = clock_->CurrentTime() + duration;
        if (schedule_next_rtcp_send_) {
            schedule_next_rtcp_send_(duration);
        }
    }

    int RTCPSender::ComputeCompoundRTCPPacket(const FeedbackState& feedback_state,
                                              RTCPPacketType packet_type,
                                              size_t nack_size,
                                              const uint16_t* nack_list,
                                              PacketSender& sender)
    {
        SetFlag(packet_type);

        // 当没有任何RTP包发送时，需要阻止SR的发送
        bool can_calculate_rtp_timestamp = last_frame_capture_time_.has_value();
        if (!can_calculate_rtp_timestamp) {
            // 此时不能发送SR包
            bool send_sr_flag = ConsumeFlag(kRtcpSr);
            bool send_report_flag = ConsumeFlag(kRtcpReport);
            bool sender_report = send_sr_flag || send_report_flag;
            // 如果当前仅仅只需要发送SR包，我们直接return
            if (sender_report && AllVolatileFlagsConsumed()) {
                return 0;
            }

            // 如果还需要发送其它的RTCP包
            if (sending_ && mode_ == webrtc::RtcpMode::kCompound) {
                // 复合包模式下，发送任何RTCP包都必须携带SR包，
                // 此时又没有发送任何RTP包，不能发送当前的RTCP包
                return -1;
            }
        }

        RtcpContext context(feedback_state, clock_->CurrentTime());

        PrepareReport(feedback_state);

        // 遍历flag，根据rtcp类型，构造对应的rtcp包
        auto it = report_flags_.begin();
        while (it != report_flags_.end()) {
            uint32_t rtcp_packet_type = it->type;
            if (it->is_volatile) {
                report_flags_.erase(it++);
            }
            else {
                it++;
            }

            // 通过rtcp类型，找到对应的处理函数
            auto builder_it = builders_.find(rtcp_packet_type);
            if (builder_it == builders_.end()) {
                RTC_LOG(LS_WARNING) << "could not find builder for rtcp_packet_type:"
                                    << rtcp_packet_type;
            }
            else {
                BuilderFunc func = builder_it->second;
                (this->*func)(context, sender);
            }
        }

        return 0;
    }

    void RTCPSender::SetFlag(uint32_t type) {
        report_flags_.insert(ReportFlag(type, true));
    }

    bool RTCPSender::ConsumeFlag(uint32_t type, bool forced) {
        auto it = report_flags_.find(ReportFlag(type, false));
        if (it == report_flags_.end()) {
            return false;
        }

        if (it->is_volatile || forced) {
            report_flags_.erase(it);
        }

        return true;
    }

    bool RTCPSender::AllVolatileFlagsConsumed() {
        for (auto flag : report_flags_) {
            if (flag.is_volatile) {
                return false;
            }
        }
        return true;
    }

    void RTCPSender::PrepareReport(const FeedbackState& feedback_state) {
        bool generate_report = true;

        ConsumeFlag(kRtcpReport, false);
        SetFlag(sending_ ? kRtcpSr : kRtcpRr);

        if (generate_report) {
            // 设置下一次发送报告的时间
            int minimal_interval_ms = report_interval_ms_.ms();
            webrtc::TimeDelta time_to_next = webrtc::TimeDelta::Millis(
                    random_.Rand(minimal_interval_ms * 1 / 2, minimal_interval_ms * 3 / 2)
            );
            SetNextRtcpSendEvaluationDuration(time_to_next);
        }
    }

    void RTCPSender::BuildSr(const RtcpContext& context, PacketSender& sender) {
        // 计算当前时间的rtp_timestamp
        // last_rtp_timestamp_ -> last_frame_capture_time_
        //            ?        -> context.now
        uint32_t rtp_timestamp = last_rtp_timestamp_ +
                                 ((context.now.us() + 500) / 1000 - last_frame_capture_time_->ms()) *
                                 (clock_rate_ / 1000);

        rtcp::SenderReport sr;
        sr.SetSenderSsrc(ssrc_);
        sr.SetNtpTime(clock_->ConvertTimestampToNtpTime(context.now));
        sr.SetRtpTimestamp(rtp_timestamp);
        sr.SetSendPacketCount(context.feedback_state.packets_sent);
        sr.SetSendPacketOctet(context.feedback_state.media_bytes_sent);

        sender.Append(sr);
    }

} // namespace xrtc

