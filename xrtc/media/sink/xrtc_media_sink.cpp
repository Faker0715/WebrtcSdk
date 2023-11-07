//
// Created by faker on 2023/6/17.
//

#include "xrtc/media/sink/xrtc_media_sink.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "xrtc/base/xrtc_global.h"
#include "xrtc/base/xrtc_http.h"
#include "xrtc/base/xrtc_json.h"
#include "xrtc/base/xrtc_utils.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"

namespace xrtc {

    XRTCMediaSink::XRTCMediaSink(MediaChain* media_chain) :
            media_chain_(media_chain),
            audio_in_pin_(std::make_unique<InPin>(this)),
            video_in_pin_(std::make_unique<InPin>(this)),
            pc_(std::make_unique<PeerConnection>())
    {
        MediaFormat audio_fmt;
        audio_fmt.media_type = MainMediaType::kMainTypeAudio;
        audio_fmt.sub_fmt.audio_fmt.type = SubMediaType::kSubTypeOpus;
        audio_in_pin_->set_format(audio_fmt);

        MediaFormat video_fmt;
        video_fmt.media_type = MainMediaType::kMainTypeVideo;
        video_fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeH264;
        video_in_pin_->set_format(video_fmt);

        XRTCGlobal::Instance()->http_manager()->AddObject(this);

        pc_->SignalConnectionState.connect(this, &XRTCMediaSink::OnConnectionState);
        pc_->SignalNetworkInfo.connect(this, &XRTCMediaSink::OnNetworkInfo);
    }

    XRTCMediaSink::~XRTCMediaSink() {
        XRTCGlobal::Instance()->http_manager()->RemoveObject(this);
    }

    bool XRTCMediaSink::Start() {
        // 解析Url
        if (!ParseUrl(url_, protocol_, host_, action_, request_params_)) {
            return false;
        }

        if (action_ != "push" || request_params_["uid"].empty() || request_params_["streamName"].empty()) {
            RTC_LOG(LS_WARNING) << "invalid url: " << url_;
            return false;
        }

        // 发送信令请求
        // 构造body
        std::stringstream body;
        body << "uid=" << request_params_["uid"]
             << "&streamName=" << request_params_["streamName"]
             << "&audio=1&video=1&isDtls=0&isTest=1";
        std::string url = "https://" + host_ + "/signaling/push";
        HttpRequest request(url, body.str());

        // 发送请求
        XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
            RTC_LOG(LS_INFO) << "signaling push response, url: " << reply.get_url()
                             << ", body: " << reply.get_body()
                             << ", status: " << reply.get_status_code()
                             << ", err_no: " << reply.get_errno()
                             << ", err_msg: " << reply.get_err_msg()
                             << ", response: " << reply.get_resp();

            std::string type;
            std::string sdp;

            if (!ParseReply(reply, type, sdp)) {
                if (media_chain_) {
                    media_chain_->OnChainFailed(this, XRTCError::kPushRequestOfferErr);
                }
                return;
            }

            if (pc_->SetRemoteSDP(sdp) != 0) {
                return;
            }

            RTCOfferAnswerOptions options;
            options.recv_audio = false;
            options.recv_video = false;
            std::string answer = pc_->CreateAnswer(options, request_params_["uid"]);
            SendAnswer(answer);

        }, this);

        return true;
    }

    void XRTCMediaSink::Setup(const std::string& json_config) {
        JsonValue value;
        value.FromJson(json_config);
        JsonObject jobj = value.ToObject();
        JsonObject jxrtc_media_sink = jobj["xrtc_media_sink"].ToObject();
        url_ = jxrtc_media_sink["url"].ToString();
    }

    void XRTCMediaSink::Stop() {
        RTC_LOG(LS_INFO) << "XRTCMediaSink Stop";
        // 向后台服务发送停止推流请求
        SendStop();
    }

    void XRTCMediaSink::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        // 通过网络线程，将音视频压缩后的数据发送到服务器
        XRTCGlobal::Instance()->network_thread()->PostTask(webrtc::ToQueuedTask([=]() {
            if (MainMediaType::kMainTypeVideo == frame->fmt.media_type) {
                PacketAndSendVideo(frame);
            }
            else if (MainMediaType::kMainTypeAudio == frame->fmt.media_type) {
                PacketAndSendAudio(frame);
            }
        }));
    }

    void XRTCMediaSink::OnNetworkInfo(PeerConnection*, int64_t rtt_ms,
                                      int32_t packets_lost, uint8_t fraction_lost, uint32_t jitter)
    {
        XRTCGlobal::Instance()->api_thread()->PostTask(
                webrtc::ToQueuedTask([=]() {
                    XRTCGlobal::Instance()->engine_observer()->OnNetworkInfo(
                            rtt_ms, packets_lost, fraction_lost, jitter);
                }));
    }

    void XRTCMediaSink::OnConnectionState(PeerConnection*,
                                          PeerConnectionState pc_state)
    {
        if (PeerConnectionState::kConnected == pc_state) {
            if (media_chain_) {
                media_chain_->OnChainSuccess();
            }
        }
        else if (PeerConnectionState::kFailed == pc_state) {
            if (media_chain_) {
                media_chain_->OnChainFailed(this, XRTCError::kPushIceConnectionErr);
            }
        }
    }

    bool XRTCMediaSink::ParseReply(const HttpReply& reply, std::string& type,
                                   std::string& sdp)
    {
        if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
            RTC_LOG(LS_WARNING) << "signaling response error";
            return false;
        }

        JsonValue value;
        if (!value.FromJson(reply.get_resp())) {
            RTC_LOG(LS_WARNING) << "invalid json response";
            return false;
        }

        JsonObject jobj = value.ToObject();
        int err_no = jobj["errNo"].ToInt();
        if (err_no != 0) {
            RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
            return false;
        }

        JsonObject data = jobj["data"].ToObject();
        type = data["type"].ToString();
        sdp = data["sdp"].ToString();

        if (sdp.empty()) {
            RTC_LOG(LS_WARNING) << "sdp is empty";
            return false;
        }

        return true;
    }

    void XRTCMediaSink::SendAnswer(const std::string& answer) {
        if (request_params_["uid"].empty() || request_params_["streamName"].empty()) {
            RTC_LOG(LS_WARNING) << "send answer failed, invalid url: " << url_;
            return;
        }

        // 构造body
        std::stringstream body;
        body << "uid=" << request_params_["uid"]
             << "&streamName=" << request_params_["streamName"]
             << "&type=push&isTest=1"
             << "&answer=" << HttpManager::UrlEncode(answer);

        std::string url = "https://" + host_ + "/signaling/sendanswer";
        HttpRequest request(url, body.str());
        // 发送请求
        XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
            RTC_LOG(LS_INFO) << "signaling sendanswer response, url: " << reply.get_url()
                             << ", body: " << reply.get_body()
                             << ", status: " << reply.get_status_code()
                             << ", err_no: " << reply.get_errno()
                             << ", err_msg: " << reply.get_err_msg()
                             << ", response: " << reply.get_resp();

            if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
                RTC_LOG(LS_WARNING) << "signaling sendanswer response error";
                return;
            }

            JsonValue value;
            if (!value.FromJson(reply.get_resp())) {
                RTC_LOG(LS_WARNING) << "invalid json response";
                return;
            }

            JsonObject jobj = value.ToObject();
            int err_no = jobj["errNo"].ToInt();
            if (err_no != 0) {
                RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
                return;
            }

        }, this);
    }

    void XRTCMediaSink::SendStop() {
        // 发送停止推流的信令请求
        // https://www.str2num.com/signaling/stoppush?uid=xxx&streamName=xxx
        // 构造body
        std::stringstream body;
        body << "uid=" << request_params_["uid"]
             << "&streamName=" << request_params_["streamName"];
        std::string url = "https://" + host_ + "/signaling/stoppush";
        HttpRequest request(url, body.str());

        // 发送请求
        XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
            RTC_LOG(LS_INFO) << "signaling stoppush response, url: " << reply.get_url()
                             << ", body: " << reply.get_body()
                             << ", status: " << reply.get_status_code()
                             << ", err_no: " << reply.get_errno()
                             << ", err_msg: " << reply.get_err_msg()
                             << ", response: " << reply.get_resp();

            if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
                RTC_LOG(LS_WARNING) << "signaling stoppush response error";
                return;
            }

            JsonValue value;
            if (!value.FromJson(reply.get_resp())) {
                RTC_LOG(LS_WARNING) << "invalid json response";
                return;
            }

            JsonObject jobj = value.ToObject();
            int err_no = jobj["errNo"].ToInt();
            if (err_no != 0) {
                RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
                return;
            }

        }, this);
    }

    void XRTCMediaSink::PacketAndSendAudio(std::shared_ptr<MediaFrame> frame) {
        pc_->SendEncodedAudio(frame);
    }

    void XRTCMediaSink::PacketAndSendVideo(std::shared_ptr<MediaFrame> frame) {
        pc_->SendEncodedImage(frame);
    }

} // namespace xrtc