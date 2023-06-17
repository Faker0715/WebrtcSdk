//
// Created by faker on 2023/6/17.
//

#include "xrtc/media/sink/xrtc_media_sink.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_http.h"
#include "xrtc/base/xrtc_global.h"
#include "xrtc/base/xrtc_json.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/base/xrtc_utils.h"

namespace xrtc {

    XRTCMediaSink::XRTCMediaSink(MediaChain* media_chain) :
            media_chain_(media_chain),
            video_in_pin_(std::make_unique<InPin>(this))

    {
        MediaFormat video_fmt;
        video_fmt.media_type = MainMediaType::kMainTypeVideo;
        video_fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeH264;
        video_in_pin_->set_format(video_fmt);
        XRTCGlobal::Instance()->http_manager()->AddObject(this);
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
             << "&audio=1&video=1&isDtls=0";
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
    }

    void XRTCMediaSink::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
    }



} // namespace xrtc