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
    class MediaFrame;
    class XRTCPreview;
    class XRTCRender;
    class XRTCPusher;

    enum class XRTCError {
        kNoErr = 0,
        kVideoCaptureCreateErr = -1000,
        kVideoNoCapabilitiesErr,
        kVideoNoBestCapabilitiesErr,
        kVideoStartCaptureErr,
        kPreviewNoVideoSourceErr,
        kChainConnectErr,
        kChainStartErr,
        kPushNoVideoSourceErr
    };

    class IXRTCConsumer {
    public:
        virtual ~IXRTCConsumer() {};

        virtual void OnFrame(std::shared_ptr<MediaFrame>) = 0;
    };

    class IMediaSource {
    public:
        virtual ~IMediaSource() {};

        virtual void Setup(const std::string& json_config) = 0;
        virtual void Start() = 0;

        virtual void Stop() = 0;

        virtual void Destroy() = 0;

        virtual void AddConsumer(IXRTCConsumer *consumer) = 0;

        virtual void RemoveConsumer(IXRTCConsumer *consumer) = 0;
    };
    class IVideoSource : public IMediaSource {
    };


    class XRTC_API XRTCEngineObserver{
    public:
        virtual void OnVideoSourceSuccess(IVideoSource* source) {};
        virtual void OnVideoSourceFailed(IVideoSource* source, XRTCError error) {};

        virtual void OnPreviewSuccess(XRTCPreview* preview) {};
        virtual void OnPreviewFailed(XRTCPreview* preview, XRTCError error) {};
        virtual void OnNetworkInfo(int64_t rtt_ms,int32_t packets_lost,uint8_t fraction_lost,uint32_t jitter) {};

    };


    class XRTC_API XRTCEngine {
    public:
        static void Init(XRTCEngineObserver *observer);

        static uint32_t GetCameraCount();

        static int32_t GetCameraInfo(int index, std::string &device_name,
                                     std::string &device_id);

        static IVideoSource *CreateCamSource(const std::string &cam_id);

        static XRTCRender* CreateRender(void* canvas);
        static XRTCPreview* CreatePreview(IVideoSource* video_source, XRTCRender* render);
        static XRTCPusher* CreatePusher(IVideoSource *video_source);



    };

}

#endif
