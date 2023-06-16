//
// Created by faker on 2023/6/16.
//

#ifndef XRTCSDK_X264_ENCODER_FILTER_H
#define XRTCSDK_X264_ENCODER_FILTER_H

#include <queue>
#include <mutex>
#include "xrtc/media/base/media_chain.h"

extern "C" {
#include <x264.h>
};
namespace xrtc {
    // 暴露几个必要的参数进行设置
    struct X264EncoderPararm{
        // 编码速率
        std::string preset = "veryfast";
        // 编码场景
        std::string tune = "zerolatency";
        // profile
        std::string profile = "baseline";

        std::string rate_control = "CRF";
        int bitrate = 1000;
        int max_bitrate = 1500;
        int buffer_size = 1500;
        int cf = 25;

        int width = 640;
        int height = 480;
        int fps = 30;
        int gop = 60; // 2s
    };

    class X264EncoderFilter : public MediaObject {
    public:
        X264EncoderFilter();

        ~X264EncoderFilter() override;

        bool Start() override;

        void Stop() override;

        void OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) override;

        std::vector<InPin *> GetAllInPins() override {
            return std::vector<InPin *>{in_pin_.get()};
        };

        std::vector<OutPin *> GetAllOutPins() override {
            return std::vector<OutPin *>{out_pin_.get()};
        };
    private:
        bool InitEncoder();
        void ReleaseEncoder();

    private:
        std::unique_ptr<InPin> in_pin_;
        std::unique_ptr<OutPin> out_pin_;
        std::queue<std::shared_ptr<MediaFrame>> frame_queue_;
        std::mutex frame_queue_mutex_;
        std::atomic<bool> running_{false};
        std::thread *encoder_thread_ = nullptr;
        std::condition_variable cond_var_;
        X264EncoderPararm encoder_param_;
        x264_param_t* x264_param_ = nullptr;
    };


}


#endif //XRTCSDK_X264_ENCODER_FILTER_H
