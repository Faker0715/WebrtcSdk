//
// Created by faker on 2023/6/29.
//

#ifndef XRTCSDK_AUDIO_PROCESSING_FILTER_H
#define XRTCSDK_AUDIO_PROCESSING_FILTER_H


#include <modules/audio_processing/include/audio_processing.h>
#include <common_audio/resampler/include/push_resampler.h>

#include "xrtc/media/base/media_chain.h"

namespace xrtc {

    class AudioProcessingFilter : public MediaObject {
    public:
        AudioProcessingFilter();
        ~AudioProcessingFilter() override;

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
        std::unique_ptr<InPin> in_pin_;
        std::unique_ptr<OutPin> out_pin_;
        rtc::scoped_refptr<webrtc::AudioProcessing> audio_processing_;
        int encoder_clock_rate_ = 48000;
        size_t encoder_channels_ = 2;
        webrtc::PushResampler<int16_t> capture_resampler_;
    };

} // namespace xrtc

#endif //XRTCSDK_AUDIO_PROCESSING_FILTER_H
