//
// Created by faker on 2023/6/29.
//

#ifndef XRTCSDK_OPUS_ENCODER_FILTER_H
#define XRTCSDK_OPUS_ENCODER_FILTER_H


#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

#include "xrtc/media/base/media_chain.h"

class OpusEncoder;

namespace xrtc {

    class OpusEncoderFilter : public MediaObject {
    public:
        OpusEncoderFilter();
        ~OpusEncoderFilter() override;

        bool Start() override;
        void Setup(const std::string& json_config) override;
        void Stop() override;
        void OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) override;
        std::vector<InPin*> GetAllInPins() override {
            return std::vector<InPin*>({ in_pin_.get() });
        }

        std::vector<OutPin*> GetAllOutPins() override {
            return std::vector<OutPin*>({ out_pin_.get() });
        }

    private:
        OpusEncoder* CreateEncoder();

    private:
        std::unique_ptr<InPin> in_pin_;
        std::unique_ptr<OutPin> out_pin_;
        std::thread* encoder_thread_ = nullptr;
        std::atomic<bool> running_{ false };
        int opus_app_;
        size_t sample_rate_hz_ = 48000;
        size_t channels_ = 1;
        uint32_t bitrate_ = 48000; // 48kbps
        std::queue<std::shared_ptr<MediaFrame>> frame_queue_;
        std::mutex frame_queue_mtx_;
        std::condition_variable cond_var_;
    };

} // namespace xrtc

#endif //XRTCSDK_OPUS_ENCODER_FILTER_H
