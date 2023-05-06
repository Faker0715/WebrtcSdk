#ifndef XRTCSDK_XRTC_XRTC_H_
#define XRTCSDK_XRTC_XRTC_H_
#ifdef XRTC_STATIC
#define XRTC_API
#else
#ifdef XRTC_API_EXPORT
#if defined(_MSC_VER)
#define XRTC_API __declspec(dllexport)
#else
#define XRTC_API
#endif
#else
#if defined(_MSC_VER)
#define XRTC_API __declspec(dllimport)
#else
#define XRTC_API
#endif
#endif
#endif
#include <string>
#include <memory>
namespace xrtc {
    class MediaFrame ;
    class IXRTCConsumer {
    public:
        virtual ~IXRTCConsumer() {};
        virtual void OnFrame(std::shared_ptr<MediaFrame>)  = 0;
    };

    class IVideoSource{
    public:
        virtual ~IVideoSource() {};
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Destroy() = 0;
        virtual void AddConsumer(IXRTCConsumer* consumer) = 0;
        virtual void RemoveConsumer(IXRTCConsumer* consumer) = 0;
    };
class XRTC_API XRTCEngine {
public:
    static void Init();
    static uint32_t GetCameraCount();
    static int32_t GetCameraInfo(int index,std::string& device_name,
                                 std::string& device_id);


};

}

#endif