//
// Created by faker on 2023/6/18.
//

#include "xrtc/rtc/pc/session_description.h"

#include <sstream>

namespace xrtc {

    const char kMediaProtocolDtlsSavpf[] = "UDP/TLS/RTP/SAVPF";
    const char kMediaProtocolSavpf[] = "RTP/SAVPF";

    void ContentGroup::AddContentName(const std::string& content_name) {
        if (!HasContentName(content_name)) {
            content_names_.push_back(content_name);
        }
    }

    bool ContentGroup::HasContentName(const std::string& content_name) {
        for (auto name : content_names_) {
            if (content_name == name) {
                return true;
            }
        }
        return false;
    }

    AudioContentDescription::AudioContentDescription() {
        auto codec = std::make_shared<AudioCodecInfo>();
        codec->id = 111;
        codec->name = "opus";
        codec->clockrate = 48000;
        codec->channels = 2;

//         添加feedback param
        codec->feedback_param.push_back(FeedbackParam("transport-cc"));

//         添加codec param
        codec->codec_param["minptime"] = "10";
        codec->codec_param["useinbandfec"] = "1";

        codecs_.push_back(codec);
    }

    VideoContentDescription::VideoContentDescription() {
        auto codec = std::make_shared<VideoCodecInfo>();
        codec->id = 107;
        codec->name = "H264";
        codec->clockrate = 90000;

//         添加feedback param
        codec->feedback_param.push_back(FeedbackParam("goog-remb"));
        codec->feedback_param.push_back(FeedbackParam("transport-cc"));
        codec->feedback_param.push_back(FeedbackParam("ccm", "fir"));
        codec->feedback_param.push_back(FeedbackParam("nack"));
        codec->feedback_param.push_back(FeedbackParam("nack", "pli"));

//         添加codec param
        // 支持不同的编码等级
        codec->codec_param["level-asymmetry-allowed"] = "1";
        codec->codec_param["packetization-mode"] = "1";
        codec->codec_param["profile-level-id"] = "42e01f";

        codecs_.push_back(codec);

//         重传的codec
        auto codec_rtx = std::make_shared<VideoCodecInfo>();
        codec_rtx->id = 99;
        codec_rtx->name = "rtx";
        codec_rtx->clockrate = 90000;

        // 视频id
        codec_rtx->codec_param["apt"] = std::to_string(codec->id);

        codecs_.push_back(codec_rtx);
    }


    SessionDescription::SessionDescription(SdpType type) :
            sdp_type_(type)
    {
    }

    SessionDescription::~SessionDescription() {
    }

    const ContentGroup* SessionDescription::GetGroupByName(const std::string& name) {
        for (const ContentGroup& group : content_groups_) {
            if (group.semantics() == name) {
                return &group;
            }
        }
        return nullptr;
    }

    std::shared_ptr<TransportDescription> SessionDescription::GetTransportInfo(
            const std::string transport_name)
    {
        for (auto td : transport_info_) {
            if (td->mid == transport_name) {
                return td;
            }
        }
        return nullptr;
    }

    void SessionDescription::AddTransportInfo(std::shared_ptr<TransportDescription> td) {
        transport_info_.push_back(td);
    }

    void SessionDescription::AddTransportInfo(const std::string& mid,
                                              const ice::IceParameters& ice_param)
    {
        auto td = std::make_shared<TransportDescription>();
        td->mid = mid;
        td->ice_ufrag = ice_param.ice_ufrag;
        td->ice_pwd = ice_param.ice_pwd;

        transport_info_.push_back(td);
    }

    bool SessionDescription::IsBundle(const std::string& mid) {
        auto content_group = GetGroupByName("BUNDLE");
        if (!content_group || content_group->content_names().empty()) {
            return false;
        }

        for (auto content_name : content_group->content_names()) {
            if (mid == content_name) {
                return true;
            }
        }

        return false;
    }

    std::string SessionDescription::GetFirstBundleId() {
        auto group = GetGroupByName("BUNDLE");
        if (!group || group->content_names().empty()) {
            return "";
        }

        return group->content_names()[0];
    }

    static void AddRtcpFbLine(std::shared_ptr<CodecInfo> codec,
                              std::stringstream& ss)
    {
        for (auto param : codec->feedback_param) {
            ss << "a=rtcp-fb:" << codec->id << " " << param.id();
            if (!param.param().empty()) {
                ss << " " << param.param();
            }
            ss << "\r\n";
        }
    }

    static void AddFmtpLine(std::shared_ptr<CodecInfo> codec,
                            std::stringstream& ss)
    {
        if (!codec->codec_param.empty()) {
            // a=fmtp:107            level-asymmetry-allowed=1;packetization-
            // mode=1;profile-level-id=42e01f
            ss << "a=fmtp:" << codec->id << " ";
            std::string data = "";
            for (auto param : codec->codec_param) {
                data += (";" + param.first + "=" + param.second);
            }

//            ;key1=value1;key2=values
            data = data.substr(1);
            ss << data << "\r\n";
        }
    }

    static void BuildRtpMap(MediaContentDescription* media_content,
                            std::stringstream& ss)
    {
        for (auto codec : media_content->codecs()) {
            ss << "a=rtpmap:" << codec->id << " " << codec->name << "/"
               << codec->clockrate;
            if (media_content->type() == webrtc::MediaType::AUDIO) {
                auto audio_codec = codec->AsAudio();
                ss << "/" << audio_codec->channels;
            }
            ss << "\r\n";

            // 添加编解码参数
            AddRtcpFbLine(codec, ss);
            AddFmtpLine(codec, ss);
        }
    }

    static std::string GetDirection(RtpDirection direction) {
        switch (direction) {
            case RtpDirection::kSendRecv:
                return "sendrecv";
            case RtpDirection::kSendOnly:
                return "sendonly";
            case RtpDirection::kRecvOnly:
                return "recvonly";
            default:
                return "inactive";
        }
    }

    static void BuildSsrc(MediaContentDescription* media_content,
                          std::stringstream& ss)
    {
        for (auto stream : media_content->streams()) {
//             生成ssrc group
            for (auto group : stream.ssrc_groups) {
                if (group.ssrcs.empty()) {
                    continue;
                }

                ss << "a=ssrc-group:FID";
                for (auto ssrc : group.ssrcs) {
                    ss << " " << ssrc;
                }
                ss << "\r\n";
            }

//             生成ssrc
            for (auto ssrc : stream.ssrcs) {
                ss << "a=ssrc:" << ssrc << " cname:" << stream.cname << "\r\n";
                ss << "a=ssrc:" << ssrc << " msid:" << stream.stream_id
                   << " " << stream.id << "\r\n";
            }
        }

    }

    std::string SessionDescription::ToString() {
        std::stringstream ss;
        // version
        ss << "v=0\r\n";
        // session origin
        // RFC 4566
        // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
        ss << "o=- 0 2 IN IP4 127.0.0.1\r\n";
        // session name
        ss << "s=-\r\n";
        // time description
        ss << "t=0 0\r\n";

        // 生成BUNDLE信息
        auto answer_bundle = GetGroupByName("BUNDLE");
        if (answer_bundle && !(answer_bundle->content_names().empty())) {
            ss << "a=group:BUNDLE";
            for (auto content_name : answer_bundle->content_names()) {
                ss << " " << content_name;
            }
            ss << "\r\n";
        }

        ss << "a=msid-semantic: WMS\r\n";

        for (auto content : contents_) {
            // 生成m行
            // RFC 4566
            // m=<media> <port> <proto> <fmt>
            std::string fmt;
            for (auto codec : content->codecs()) {
                fmt.append(" ");
                fmt.append(std::to_string(codec->id));
            }

            ss << "m=" << content->mid() << " 9 " << kMediaProtocolSavpf
               << fmt << "\r\n";
            ss << "c=IN IP4 0.0.0.0\r\n";
            ss << "a=rtcp:9 IN IP4 0.0.0.0\r\n";

            auto td = GetTransportInfo(content->mid());
            if (td) {
                ss << "a=ice-ufrag:" << td->ice_ufrag << "\r\n";
                ss << "a=ice-pwd:" << td->ice_pwd << "\r\n";
            }

            ss << "a=mid:" << content->mid() << "\r\n";
            ss << "a=" << GetDirection(content->direction()) << "\r\n";
            if (content->rtcp_mux()) {
                ss << "a=rtcp-mux" << "\r\n";
            }

            BuildRtpMap(content.get(), ss);
            BuildSsrc(content.get(), ss);
        }

        return ss.str();
    }

} // namespace xrtc

