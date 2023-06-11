//
// Created by faker on 2023/5/8.
//

#ifndef XRTCSDK_MEDIA_FRAME_H
#define XRTCSDK_MEDIA_FRAME_H

#include <cstring>
#include <cstdint>

namespace xrtc {

    enum class MainMediaType {
        kMainTypeCommon,
        kMainTypeAudio,
        kMainTypeVideo,
        kMainTypeData,
    };
    enum class SubMediaType {
        kSubTypeCommon,
        kSubTypeI420
    };
    struct AudioFormat {
        SubMediaType type;
    };
    struct VideoFormat {
        SubMediaType type;
        int width;
        int height;
        bool idr ;
    };

    class MediaFormat {
    public:
        MainMediaType media_type;
        union{
            VideoFormat video_fmt;
            AudioFormat audio_fmt;;
        } sub_fmt;
    };

    class MediaFrame {
    public:
        MediaFrame(int size) : max_size(size){
            memset(data, 0, sizeof(data));
            memset(data_len, 0, sizeof(data_len));
            memset(stride, 0, sizeof(stride));
            data[0] = new char[size];
            data_len[0] = size;
        }
        ~MediaFrame(){
            if(data[0]){
                delete[] data[0];
                data[0] = nullptr;
            }
        }
    public:
        int max_size;
        MediaFormat fmt;
        // 4个平面
        char *data[4];
        // 每个平面的字节数
        int data_len[4];
        int stride[4];
        uint32_t ts = 0;
    };
}


#endif //XRTCSDK_MEDIA_FRAME_H