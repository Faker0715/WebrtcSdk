//
// Created by faker on 2023/6/18.
//
#include "xrtc/rtc/pc/peer_connection.h"

#include <vector>

#include <rtc_base/logging.h>
#include <rtc_base/string_encode.h>
#include <rtc_base/helpers.h>
#include <api/task_queue/default_task_queue_factory.h>
#include <ice/candidate.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"
#include "xrtc/base/xrtc_global.h"

namespace xrtc {

    const size_t RTC_PACKET_CACHE_SIZE = 2048;

    PeerConnection::PeerConnection() :
            transport_controller_(std::make_unique<TransportController>()),
            clock_(webrtc::Clock::GetRealTimeClock()),
            video_cache_(RTC_PACKET_CACHE_SIZE),
            task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
            transport_send_(std::make_unique<RtpTransportControllerSend>(clock_,
                                                                         this, task_queue_factory_.get()))
    {
        transport_controller_->SignalIceState.connect(this,
                                                      &PeerConnection::OnIceState);
        transport_controller_->SignalRtcpPacketReceived.connect(this,
                                                                &PeerConnection::OnRtcpPacketReceived);
    }

    PeerConnection::~PeerConnection() {
        XRTCGlobal::Instance()->network_thread()->Invoke<void>(RTC_FROM_HERE, [=]() {
            if (audio_send_stream_) {
                delete audio_send_stream_;
                audio_send_stream_ = nullptr;
            }

            if (video_send_stream_) {
                delete video_send_stream_;
                video_send_stream_ = nullptr;
            }
        });
    }

// a=attr_name:attr_value
    static std::string GetAttribute(const std::string& line) {
        std::vector<std::string> fields;
        size_t size = rtc::tokenize(line, ':', &fields);
        if (size != 2) {
            RTC_LOG(LS_WARNING) << "get attribute failed: " << line;
            return "";
        }

        return fields[1];
    }

    static bool ParseCandidates(MediaContentDescription* media_content,
                                const std::string& line)
    {
        if (line.find("a=candidate:") == std::string::npos) {
            return true;
        }

        std::string attr_value = GetAttribute(line);
        if (attr_value.empty()) {
            return false;
        }

        std::vector<std::string> fields;
        size_t size = rtc::tokenize(attr_value, ' ', &fields);
        if (size < 8) {
            return false;
        }

        ice::Candidate c;
        c.foundation = fields[0];
        c.component = std::atoi(fields[1].c_str());
        c.protocol = fields[2];
        c.priority = std::atoi(fields[3].c_str());
        c.port = std::atoi(fields[5].c_str());
        c.address = rtc::SocketAddress(fields[4], c.port);
        c.type = fields[7];

        media_content->AddCandidate(c);
        return true;
    }

    static bool ParseTransportInfo(TransportDescription* td,
                                   const std::string& line)
    {
        if (line.find("a=ice-ufrag") != std::string::npos) {
            td->ice_ufrag = GetAttribute(line);
            if (td->ice_ufrag.empty()) {
                return false;
            }
        } else if (line.find("a=ice-pwd") != std::string::npos) {
            td->ice_pwd = GetAttribute(line);
            if (td->ice_pwd.empty()) {
                return false;
            }
        }

        return true;
    }

    int PeerConnection::SetRemoteSDP(const std::string& sdp) {
        std::vector<std::string> fields;
        // SDP用\n, \r\n来换行的
        rtc::tokenize(sdp, '\n', &fields);
        if (fields.size() <= 0) {
            RTC_LOG(LS_WARNING) << "invalid sdp: " << sdp;
            return -1;
        }

        // 判断是否是\r\n换行
        bool is_rn = false;
        if (sdp.find("\r\n") != std::string::npos) {
            is_rn = true;
        }

        remote_desc_ = std::make_unique<SessionDescription>(SdpType::kOffer);

        std::string mid;
        auto audio_content = std::make_shared<AudioContentDescription>();
        auto video_content = std::make_shared<VideoContentDescription>();
        auto audio_td = std::make_shared<TransportDescription>();
        auto video_td = std::make_shared<TransportDescription>();

        for (auto field : fields) {
            // 如果以\r\n换行，去掉尾部的\r
            if (is_rn) {
                field = field.substr(0, field.length() - 1);
            }

            if (field.find("a=group:BUNDLE") != std::string::npos) {
                std::vector<std::string> items;
                rtc::tokenize(field, ' ', &items);
                if (items.size() > 1) {
                    ContentGroup offer_bundle("BUNDLE");
                    for (size_t i = 1; i < items.size(); ++i) {
                        offer_bundle.AddContentName(items[i]);
                    }
                    remote_desc_->AddGroup(offer_bundle);
                }
            }
            else if (field.find_first_of("m=") == 0) {
                std::vector<std::string> items;
                rtc::tokenize(field, ' ', &items);
                if (items.size() <= 2) {
                    RTC_LOG(LS_WARNING) << "parse m= failed: " << field;
                    return -1;
                }

                // m=audio/video
                mid = items[0].substr(2);
                if (mid == "audio") {
                    remote_desc_->AddContent(audio_content);
                    audio_td->mid = mid;
                }
                else if (mid == "video") {
                    remote_desc_->AddContent(video_content);
                    video_td->mid = mid;
                }
            }

            if ("audio" == mid) {
                if (!ParseCandidates(audio_content.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse candidate failed: " << field;
                    return -1;
                }

                if (!ParseTransportInfo(audio_td.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
                    return -1;
                }
            }
            else if ("video" == mid) {
                if (!ParseCandidates(video_content.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse candidate failed: " << field;
                    return -1;
                }

                if (!ParseTransportInfo(video_td.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
                    return -1;
                }
            }
        }

        remote_desc_->AddTransportInfo(audio_td);
        remote_desc_->AddTransportInfo(video_td);

        if (audio_content) {
            auto audio_codecs = audio_content->codecs();
            if (!audio_codecs.empty()) {
                audio_pt_ = audio_codecs[0]->id;
            }
        }

        if (video_content) {
            auto video_codecs = video_content->codecs();
            if (!video_codecs.empty()) {
                video_pt_ = video_codecs[0]->id;
            }

            if (video_codecs.size() > 1) {
                video_rtx_pt_ = video_codecs[1]->id;
            }
        }

        transport_controller_->SetRemoteSDP(remote_desc_.get());

        return 0;
    }

    static RtpDirection GetDirection(bool send, bool recv) {
        if (send && recv) {
            return RtpDirection::kSendRecv;
        } else if (send && !recv) {
            return RtpDirection::kSendOnly;
        } else if (!send && recv) {
            return RtpDirection::kRecvOnly;
        }
        else {
            return RtpDirection::kInactive;
        }
    }

    std::string PeerConnection::CreateAnswer(const RTCOfferAnswerOptions& options,
                                             const std::string& stream_id)
    {
        local_desc_ = std::make_unique<SessionDescription>(SdpType::kAnswer);

        ice::IceParameters ice_param = ice::IceCredentials::CreateRandomIceCredentials();
        std::string cname = rtc::CreateRandomString(16);

        if (options.send_audio || options.recv_audio) {
            auto audio_content = std::make_shared<AudioContentDescription>();
            audio_content->set_direction(GetDirection(options.send_audio, options.recv_audio));
            audio_content->set_rtcp_mux(options.use_rtcp_mux);
            local_desc_->AddContent(audio_content);
            local_desc_->AddTransportInfo(audio_content->mid(), ice_param);

            // 如果发送音频，需要创建stream
            if (options.send_audio) {
                StreamParams audio_stream;
                audio_stream.id = rtc::CreateRandomString(16);
                audio_stream.stream_id = stream_id;
                audio_stream.cname = cname;
                local_audio_ssrc_ = rtc::CreateRandomId();
                audio_stream.ssrcs.push_back(local_audio_ssrc_);
                audio_content->AddStream(audio_stream);

                CreateAudioSendStream(audio_content.get());
            }
        }

        if (options.send_video || options.recv_video) {
            auto video_content = std::make_shared<VideoContentDescription>();
            video_content->set_direction(GetDirection(options.send_video, options.recv_video));
            video_content->set_rtcp_mux(options.use_rtcp_mux);
            local_desc_->AddContent(video_content);
            local_desc_->AddTransportInfo(video_content->mid(), ice_param);

            // 如果发送视频，需要创建stream
            if (options.send_video) {
                std::string id = rtc::CreateRandomString(16);
                StreamParams video_stream;
                video_stream.id = id;
                video_stream.stream_id = stream_id;
                video_stream.cname = cname;
                local_video_ssrc_ = rtc::CreateRandomId();
                local_video_rtx_ssrc_ = rtc::CreateRandomId();
                video_stream.ssrcs.push_back(local_video_ssrc_);
                video_stream.ssrcs.push_back(local_video_rtx_ssrc_);

                // 分组
                SsrcGroup sg;
                sg.semantics = "FID";
                sg.ssrcs.push_back(local_video_ssrc_);
                sg.ssrcs.push_back(local_video_rtx_ssrc_);
                video_stream.ssrc_groups.push_back(sg);

                video_content->AddStream(video_stream);

                // 创建rtx stream
                StreamParams video_rtx_stream;
                video_rtx_stream.id = id;
                video_rtx_stream.stream_id = stream_id;
                video_rtx_stream.cname = cname;
                video_rtx_stream.ssrcs.push_back(local_video_rtx_ssrc_);
                video_content->AddStream(video_rtx_stream);

                CreateVideoSendStream(video_content.get());
            }
        }

        // 创建BUNDLE
        if (options.use_rtp_mux) {
            ContentGroup answer_bundle("BUNDLE");
            for (auto content : local_desc_->contents()) {
                answer_bundle.AddContentName(content->mid());
            }

            if (!answer_bundle.content_names().empty()) {
                local_desc_->AddGroup(answer_bundle);
            }
        }

        transport_controller_->SetLocalSDP(local_desc_.get());

        return local_desc_->ToString();
    }

    bool PeerConnection::SendEncodedAudio(std::shared_ptr<MediaFrame> frame) {
        if (pc_state_ != PeerConnectionState::kConnected) {
            return true;
        }

        // 音频数据打包成rtp的格式
        auto packet = std::make_shared<RtpPacketToSend>();
        packet->SetPayloadType(audio_pt_);
        packet->SetSequenceNumber(audio_seq_++);
        packet->SetSsrc(local_audio_ssrc_);
        packet->SetTimestamp(frame->ts);

        // 设置负载数据
        size_t payload_size = frame->data_len[0];
        uint8_t* payload = packet->AllocatePayload(payload_size);
        memcpy(payload, frame->data[0], payload_size);

        if (audio_send_stream_) {
            audio_send_stream_->OnSendingRtpFrame(frame->ts,
                                                  frame->capture_time_ms);
            audio_send_stream_->UpdateRtpStats(packet, false);
        }

        // 发送数据包
        // TODO, transport_name此处写死，后面可以换成变量
        transport_controller_->SendPacket("audio", (const char*)packet->data(),
                                          packet->size());

        return true;
    }

    bool PeerConnection::SendEncodedImage(std::shared_ptr<MediaFrame> frame) {
        if (pc_state_ != PeerConnectionState::kConnected) {
            return true;
        }

        // 视频的频率90000, 1s中90000份 1ms => 90
        uint32_t rtp_timestamp = frame->ts * 90;

        if (video_send_stream_) {
            video_send_stream_->OnSendingRtpFrame(rtp_timestamp,
                                                  frame->capture_time_ms,
                                                  frame->fmt.sub_fmt.video_fmt.idr);
        }

        RtpPacketizer::Config config;
        auto packetizer = RtpPacketizer::Create(webrtc::kVideoCodecH264,
                                                rtc::ArrayView<const uint8_t>((uint8_t*)frame->data[0], frame->data_len[0]),
                                                config);

        while (true) {
            auto single_packet = std::make_shared<RtpPacketToSend>();
            single_packet->SetPayloadType(video_pt_);
            single_packet->SetTimestamp(rtp_timestamp);
            single_packet->SetSsrc(local_video_ssrc_);

            if (!packetizer->NextPacket(single_packet.get())) {
                break;
            }

            single_packet->SetSequenceNumber(video_seq_++);
            single_packet->set_packet_type(RtpPacketMediaType::kVideo);

            if (video_send_stream_) {
                video_send_stream_->UpdateRtpStats(single_packet, false, false);
            }

            AddVideoCache(single_packet);
            // 发送数据包
            // TODO, transport_name此处写死，后面可以换成变量
            /*transport_controller_->SendPacket("audio", (const char*)single_packet->data(),
                single_packet->size());*/
            std::unique_ptr<RtpPacketToSend> packet =
                    std::make_unique<RtpPacketToSend>(*single_packet);
            transport_send_->EnqueuePacket(std::move(packet));
        }

        return true;
    }

    void PeerConnection::OnLocalRtcpPacket(webrtc::MediaType media_type,
                                           const uint8_t* data,
                                           size_t len)
    {
        // 发送RTCP复合包到网络
        if (pc_state_ != PeerConnectionState::kConnected) {
            return;
        }

        transport_controller_->SendPacket("audio", (const char*)data, len);
    }

    void PeerConnection::OnNetworkInfo(int64_t rtt_ms,
                                       int32_t packets_lost,
                                       uint8_t fraction_lost,
                                       uint32_t jitter)
    {
        SignalNetworkInfo(this, rtt_ms, packets_lost, fraction_lost, jitter);
    }

    void PeerConnection::OnNackReceived(webrtc::MediaType media_type,
                                        const std::vector<uint16_t>& nack_list)
    {
        for (auto nack_id : nack_list) {
            auto packet = FindVideoCache(nack_id);
            if (packet) {
                // 重传数据
                if (video_send_stream_) {
                    auto rtx_packet = video_send_stream_->BuildRtxPacket(packet);
                    transport_controller_->SendPacket("audio", (const char*)rtx_packet->data(),
                                                      rtx_packet->size());
                }
            }
        }
    }

    void PeerConnection::SendPacket(std::unique_ptr<RtpPacketToSend> packet) {
        if (pc_state_ != PeerConnectionState::kConnected) {
            return;
        }

        transport_controller_->SendPacket("audio", (const char*)packet->data(),
                                          packet->size());
    }

    void PeerConnection::OnIceState(TransportController*,
                                    ice::IceTransportState ice_state)
    {
        PeerConnectionState pc_state = PeerConnectionState::kNew;
        switch (ice_state) {
            case ice::IceTransportState::kNew:
                pc_state = PeerConnectionState::kNew;
                break;
            case ice::IceTransportState::kChecking:
                pc_state = PeerConnectionState::kConnecting;
                break;
            case ice::IceTransportState::kConnected:
            case ice::IceTransportState::kCompleted:
                pc_state = PeerConnectionState::kConnected;
                break;
            case ice::IceTransportState::kDisconnected:
                pc_state = PeerConnectionState::kDisconnected;
                break;
            case ice::IceTransportState::kFailed:
                pc_state = PeerConnectionState::kFailed;
                break;
            case ice::IceTransportState::kClosed:
                pc_state = PeerConnectionState::kClosed;
                break;
            default:
                break;
        }

        if (pc_state != pc_state_) {
            RTC_LOG(LS_INFO) << "peerconnection state change, from " << pc_state_
                             << "=>" << pc_state;
            pc_state_ = pc_state;
            SignalConnectionState(this, pc_state);
        }
    }

    void PeerConnection::OnRtcpPacketReceived(TransportController*,
                                              const char* data, size_t len, int64_t)
    {
        if (video_send_stream_) {
            video_send_stream_->DeliverRtcp((const uint8_t*)data, len);
        }
    }

    void PeerConnection::CreateAudioSendStream(AudioContentDescription* audio_content) {
        if (!audio_content) {
            return;
        }

        // 暂时只考虑推送一路音频
        for (auto stream : audio_content->streams()) {
            if (!stream.ssrcs.empty()) {
                AudioSendStreamConfig config;
                config.rtp.ssrc = stream.ssrcs[0];
                config.rtp.payload_type = video_pt_;
                config.rtp_rtcp_module_observer = this;

                // 用网络线程创建
                XRTCGlobal::Instance()->network_thread()->Invoke<void>(RTC_FROM_HERE,
                                                                       [=]() {
                                                                           audio_send_stream_ = new AudioSendStream(clock_, config);
                                                                       });
            }

            break;
        }
    }

    void PeerConnection::CreateVideoSendStream(VideoContentDescription* video_content) {
        if (!video_content) {
            return;
        }

        // 暂时只考虑推送一路视频
        for (auto stream : video_content->streams()) {
            if (!stream.ssrcs.empty()) {
                VideoSendStreamConfig config;
                config.rtp.ssrc = stream.ssrcs[0];
                config.rtp.payload_type = video_pt_;
                config.rtp_rtcp_module_observer = this;
                if (stream.ssrcs.size() > 1) {
                    config.rtp.rtx.ssrc = stream.ssrcs[1];
                    config.rtp.rtx.payload_type = video_rtx_pt_;
                }

                // 用网络线程创建
                XRTCGlobal::Instance()->network_thread()->Invoke<void>(RTC_FROM_HERE,
                                                                       [=]() {
                                                                           video_send_stream_ = new VideoSendStream(clock_, config);
                                                                       });
            }

            break;
        }
    }

    void PeerConnection::AddVideoCache(std::shared_ptr<RtpPacketToSend> packet) {
        uint16_t seq = packet->sequence_number();
        size_t index = seq % RTC_PACKET_CACHE_SIZE;

        if (video_cache_[index] && video_cache_[index]->sequence_number() == seq) {
            return;
        }

        video_cache_[index] = packet;
    }

    std::shared_ptr<RtpPacketToSend> PeerConnection::FindVideoCache(uint16_t seq) {
        size_t index = seq % RTC_PACKET_CACHE_SIZE;
        if (video_cache_[index] && video_cache_[index]->sequence_number() == seq) {
            return video_cache_[index];
        }

        return nullptr;
    }

} // namespace xrtc