//
// Created by faker on 2023/6/29.
//

#ifndef XRTCSDK_XRTC_AUDIO_SOURCE_H
#define XRTCSDK_XRTC_AUDIO_SOURCE_H


#ifndef XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_AUDIO_SOURCE_H_
#define XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_AUDIO_SOURCE_H_

#include "xrtc/xrtc.h"
#include "xrtc/media/base/media_chain.h"

namespace xrtc {

    class XRTCAudioSource : public IXRTCConsumer,
                            public MediaObject
    {
    public:
        XRTCAudioSource();
        ~XRTCAudioSource() override;

        bool Start() override;
        void Stop() override;
        std::vector<InPin*> GetAllInPins() override {
            return std::vector<InPin*>();
        }

        std::vector<OutPin*> GetAllOutPins() override {
            return std::vector<OutPin*>({ out_pin_.get() });
        }

        void OnFrame(std::shared_ptr<MediaFrame> frame) override;

    private:
        std::unique_ptr<OutPin> out_pin_;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_AUDIO_SOURCE_H_



#endif //XRTCSDK_XRTC_AUDIO_SOURCE_H
