//
// Created by faker on 2023/6/10.
//

#ifndef XRTCSDK_MEDIA_CHAIN_H
#define XRTCSDK_MEDIA_CHAIN_H
#include <vector>
#include <string>

#include "xrtc/xrtc.h"

namespace xrtc{
    class InPin;
    class OutPin;
    class MediaObject{
    public:
        virtual ~MediaObject() {};

        virtual bool Start() = 0;
        // 并不是都要有参数 有参数的实现
        virtual void SetUp(const std::string& json_config){};
        virtual void Stop() = 0;
        virtual void OnNewMediaFrame(std::shared_ptr<MediaFrame> frame){};
        virtual std::vector<InPin*> GetAllInPins() = 0;
        virtual std::vector<OutPin*> GetAllOutPins() = 0;
    };
    class XRTC_API MediaChain {
    public:
        MediaChain() = default;
        virtual ~MediaChain() = default;

        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Destroy() = 0;
    protected:
        void AddMediaObject(MediaObject* media_object);
        bool ConnectMediaObject(MediaObject* from, MediaObject* to);
        void SetupChain(const std::string& json_config);
        bool StartChain();

    private:
        // 所有节点集合
        std::vector<MediaObject*> media_objects_;
    };
}


#endif //XRTCSDK_MEDIA_CHAIN_H
