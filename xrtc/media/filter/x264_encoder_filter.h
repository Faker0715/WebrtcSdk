//
// Created by faker on 2023/6/16.
//

#ifndef XRTCSDK_X264_ENCODER_FILTER_H
#define XRTCSDK_X264_ENCODER_FILTER_H

#include "xrtc/media/base/media_chain.h"
namespace xrtc{

    class X264EncoderFilter : public MediaObject{
    public:
        X264EncoderFilter();
        ~X264EncoderFilter() override;

        bool Start() override;
        void Stop() override;
        void OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) override;
        std::vector<InPin*> GetAllInPins() override{
            return std::vector<InPin*>{in_pin_.get()};
        } ;
        std::vector<OutPin*> GetAllOutPins() override{
            return std::vector<OutPin*>{out_pin_.get()};
        };
    private:
        std::unique_ptr<InPin> in_pin_;
        std::unique_ptr<OutPin> out_pin_;
    };
}


#endif //XRTCSDK_X264_ENCODER_FILTER_H
