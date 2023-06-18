//
// Created by faker on 2023/6/18.
//

#include "peer_connection.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/logging.h"
#include <string>
#include <vector>
#include <ice/candidate.h>

namespace xrtc{

    PeerConnection::PeerConnection() {

    }

    PeerConnection::~PeerConnection() {

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
    int PeerConnection::SetRemoteSDP(const std::string &sdp) {
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

//                if (!ParseTransportInfo(audio_td.get(), field)) {
//                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
//                    return -1;
//                }
            }
            else if ("video" == mid) {
                if (!ParseCandidates(video_content.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse candidate failed: " << field;
                    return -1;
                }

//                if (!ParseTransportInfo(video_td.get(), field)) {
//                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
//                    return -1;
//                }
            }
        }

        remote_desc_->AddTransportInfo(audio_td);
        remote_desc_->AddTransportInfo(video_td);

        if (video_content) {
//            auto video_codecs = video_content->codecs();
//            if (!video_codecs.empty()) {
//                video_pt_ = video_codecs[0]->id;
//            }

//            if (video_codecs.size() > 1) {
//                video_rtx_pt_ = video_codecs[1]->id;
//            }
        }

//        transport_controller_->SetRemoteSDP(remote_desc_.get());

        return 0;

    }

    std::string PeerConnection::CreateAnswer(const RTCOfferAnswerOptions &options, const std::string &stream_id) {
        return nullptr;

    }

}