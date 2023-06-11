//
// Created by faker on 2023/6/11.
//

#ifndef XRTCSDK_XRTC_VIDEO_SOURCE_H
#define XRTCSDK_XRTC_VIDEO_SOURCE_H

#include "xrtc/xrtc.h"
#include "xrtc/media/base/media_chain.h"
#include <memory>

namespace xrtc {
    class OutPin;
    class XRTCVideoSource : public IXRTCConsumer, public MediaObject {
    public:
        XRTCVideoSource();

        ~XRTCVideoSource() override;

        bool Start() override;

        void Stop() override;

        void OnFrame(std::shared_ptr<MediaFrame> frame) override;

        std::vector<InPin*> GetAllInPins() override{
            return std::vector<InPin*>();
        }

        std::vector<OutPin*> GetAllOutPins() override{
            return std::vector<OutPin*>({out_pin_.get()});
        }

    private:
        std::unique_ptr<OutPin> out_pin_;


    };
}


#endif //XRTCSDK_XRTC_VIDEO_SOURCE_H
