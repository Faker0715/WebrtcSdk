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
    struct X264EncoderParam {
    // 编码速率
    std::string preset = "veryfast";
    // 编码场景
    std::string tune = "zerolatency";
    // profile
    std::string profile = "baseline";
    // 码率控制，CQP、CRF、ABR
    std::string rate_control = "CRF";
    // 目标码率, 单位kbps
    int bitrate = 1000;
    int max_bitrate = 1500;
    int buffer_size = 1500;
    int cf = 25;
    // 图像的宽高
    int width = 640;
    int height = 480;
    // 帧率
    int fps = 30;
    // GOP
    int gop = 60; // 2s
};

class X264EncoderFilter : public MediaObject {
public:
    X264EncoderFilter();
    ~X264EncoderFilter() override;

    // MediaObject
    bool Start() override;
    //void Setup(const std::string& /*json_config*/) {}
    void Stop() override;
    void OnNewMediaFrame(std::shared_ptr<MediaFrame>) override;
    std::vector<InPin*> GetAllInPins() override {
        return std::vector<InPin*>({ in_pin_.get() });
    }
    std::vector<OutPin*> GetAllOutPins() override {
        return std::vector<OutPin*>({ out_pin_.get() });
    }

private:
    bool InitEncoder();
    void ReleaseEncoder();
    bool Encode(std::shared_ptr<MediaFrame> frame,
        std::shared_ptr<MediaFrame>& out_frame);

private:
    std::unique_ptr<InPin> in_pin_;
    std::unique_ptr<OutPin> out_pin_;
    std::queue<std::shared_ptr<MediaFrame>> frame_queue_;
    std::mutex frame_queue_mtx_;
    std::atomic<bool> running_{ false };
    std::thread* encode_thread_ = nullptr;
    std::condition_variable cond_var_;

    X264EncoderParam encoder_param_;
    x264_param_t* x264_param_ = nullptr;
    x264_t* x264_ = nullptr;
    x264_picture_t* x264_picture_ = nullptr;
};


}


#endif //XRTCSDK_X264_ENCODER_FILTER_H
