//
// Created by faker on 2023/6/17.
//

#ifndef XRTCSDK_XRTC_MEDIA_SINK_H
#define XRTCSDK_XRTC_MEDIA_SINK_H


#include <string>
#include <map>

#include "xrtc/media/base/media_chain.h"
#include "xrtc/rtc/pc/peer_connection.h"

namespace xrtc {

    class InPin;

    class OutPin;

    class HttpReply;

    class XRTCMediaSink : public MediaObject,
                          public sigslot::has_slots<> {
    public:
        XRTCMediaSink(MediaChain *media_chain);

        ~XRTCMediaSink() override;

        // MediaObject
        bool Start() override;

        void Setup(const std::string & /*json_config*/) override;

        void Stop() override;

        void OnNewMediaFrame(std::shared_ptr<MediaFrame>) override;

        std::vector<InPin *> GetAllInPins() override {
            return std::vector<InPin *>({video_in_pin_.get()});
        }

        std::vector<OutPin *> GetAllOutPins() override {
            return std::vector<OutPin *>();
        }
    private:
        void OnNetworkInfo(PeerConnection*,int64_t rtt_ms,int32_t packets_lost,uint8_t fraction_lost,uint32_t jitter);
        void OnConnectionState(PeerConnection*,PeerConnectionState state);


    private:
        bool ParseReply(const HttpReply &reply, std::string &type, std::string &sdp);

        void SendAnswer(const std::string &answer);

        void SendStop();
        void PacketAndSendVideo(std::shared_ptr<MediaFrame> frame);


    private:
        MediaChain *media_chain_;
        std::unique_ptr<InPin> video_in_pin_;
        std::string url_;
        std::string protocol_;
        std::string host_;
        std::string action_;
        std::map<std::string, std::string> request_params_;
        std::unique_ptr<PeerConnection> pc_;
    };

} // namespace xrtc

#endif //XRTCSDK_XRTC_MEDIA_SINK_H
