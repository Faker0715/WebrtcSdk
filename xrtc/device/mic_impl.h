//
// Created by faker on 2023/6/28.
//

#ifndef XRTCSDK_MIC_IMPL_H
#define XRTCSDK_MIC_IMPL_H


#include <mutex>

#include <rtc_base/thread.h>
#include <modules/audio_device/include/audio_device.h>

#include "xrtc/xrtc.h"

namespace xrtc {

    class MicImpl : public IAudioSource,
                    public webrtc::AudioTransport
    {
    public:
        void Start() override;
        void Setup(const std::string& json_config) override;
        void Stop() override;
        void Destroy() override;
        void AddConsumer(IXRTCConsumer* consumer) override;
        void RemoveConsumer(IXRTCConsumer* consumer) override;

        int32_t RecordedDataIsAvailable(const void* audioSamples,
                                        const size_t nSamples,
                                        const size_t nBytesPerSample,
                                        const size_t nChannels,
                                        const uint32_t samplesPerSec,
                                        const uint32_t totalDelayMS,
                                        const int32_t clockDrift,
                                        const uint32_t currentMicLevel,
                                        const bool keyPressed,
                                        uint32_t& newMicLevel) override;  // NOLINT

        // Implementation has to setup safe values for all specified out parameters.
        int32_t NeedMorePlayData(const size_t nSamples,
                                 const size_t nBytesPerSample,
                                 const size_t nChannels,
                                 const uint32_t samplesPerSec,
                                 void* audioSamples,
                                 size_t& nSamplesOut,  // NOLINT
                                 int64_t* elapsed_time_ms,
                                 int64_t* ntp_time_ms) override { return -1; } // NOLINT

        // Method to pull mixed render audio data from all active VoE channels.
        // The data will not be passed as reference for audio processing internally.
        void PullRenderData(int bits_per_sample,
                            int sample_rate,
                            size_t number_of_channels,
                            size_t number_of_frames,
                            void* audio_data,
                            int64_t* elapsed_time_ms,
                            int64_t* ntp_time_ms) override {}

    private:
        MicImpl(const std::string& mic_id);
        ~MicImpl() override;

        friend class XRTCEngine;

    private:
        std::string mic_id_; // 麦克风的id
        rtc::Thread* api_thread_;
        bool has_start_ = false;
        std::vector<IXRTCConsumer*> consumer_list_;
        std::mutex mtx_;
        uint32_t timestamp_ = 0;
    };

} // namespace xrtc

#endif //XRTCSDK_MIC_IMPL_H
