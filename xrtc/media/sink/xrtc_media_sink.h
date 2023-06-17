//
// Created by faker on 2023/6/17.
//

#ifndef XRTCSDK_XRTC_MEDIA_SINK_H
#define XRTCSDK_XRTC_MEDIA_SINK_H


#include <string>
#include <map>

#include "xrtc/media/base/media_chain.h"

namespace xrtc {

    class InPin;
    class OutPin;
    class HttpReply;

    class XRTCMediaSink : public MediaObject
    {
    public:
        XRTCMediaSink(MediaChain* media_chain);
        ~XRTCMediaSink() override;

        // MediaObject
        bool Start() override;
        void Setup(const std::string& /*json_config*/) override;
        void Stop() override;
        void OnNewMediaFrame(std::shared_ptr<MediaFrame>) override;
        std::vector<InPin*> GetAllInPins() override {
            return std::vector<InPin*>({ video_in_pin_.get() });
        }

        std::vector<OutPin*> GetAllOutPins() override {
            return std::vector<OutPin*>();
        }


    private:
        bool ParseReply(const HttpReply& reply, std::string& type, std::string& sdp);
        void SendAnswer(const std::string& answer);

    private:
        MediaChain* media_chain_;
        std::unique_ptr<InPin> video_in_pin_;
        std::string url_;
        std::string protocol_;
        std::string host_;
        std::string action_;
        std::map<std::string, std::string> request_params_;
    };

} // namespace xrtc

#endif //XRTCSDK_XRTC_MEDIA_SINK_H