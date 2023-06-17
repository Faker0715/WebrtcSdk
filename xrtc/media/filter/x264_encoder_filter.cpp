//
// Created by faker on 2023/6/16.
//

#include "x264_encoder_filter.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/media/base/out_pin.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"

namespace xrtc {


    X264EncoderFilter::X264EncoderFilter() : in_pin_(std::make_unique<InPin>(this)),
                                             out_pin_(std::make_unique<OutPin>(this)) {
        MediaFormat fmt_in;
        fmt_in.media_type = MainMediaType::kMainTypeVideo;
        fmt_in.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
        in_pin_->set_format(fmt_in);

        MediaFormat fmt_out;
        fmt_out.media_type = MainMediaType::kMainTypeVideo;
        fmt_out.sub_fmt.video_fmt.type = SubMediaType::kSubTypeH264;


    }

    X264EncoderFilter::~X264EncoderFilter() {

    }

    bool X264EncoderFilter::Start() {
        RTC_LOG(LS_INFO) << "X264EncoderFilter Start";
        if (running_) {
            RTC_LOG(LS_WARNING) << "X264EncoderFilter already running";
            return true;
        }
        running_ = true;
        encode_thread_ = new std::thread([=]() {
            rtc::SetCurrentThreadName("x264_encoder_thread");
            RTC_LOG(LS_INFO) << "x264_encoder_thread start";
            int frame_queue_size = 0;

            if (!InitEncoder()) {
                RTC_LOG(LS_ERROR) << "x264 init failed";
                ReleaseEncoder();
                return;
            }
            while (running_) {
                std::shared_ptr<MediaFrame> frame;
                {
                    std::unique_lock<std::mutex> auto_lock(frame_queue_mtx_);
                    frame_queue_size = frame_queue_.size();
                    if (frame_queue_.size() > 0) {
                        frame = frame_queue_.front();
                        frame_queue_.pop();
                    }
                    if (!frame) {
                        cond_var_.wait(auto_lock);
                        continue;
                    }
                }

                // 判断图像的宽高是否发生了变化，如果发生了变化，需要重新初始化编码器
                if (encoder_param_.width != frame->fmt.sub_fmt.video_fmt.width ||
                    encoder_param_.height != frame->fmt.sub_fmt.video_fmt.height)
                {
                    encoder_param_.width = frame->fmt.sub_fmt.video_fmt.width;
                    encoder_param_.height = frame->fmt.sub_fmt.video_fmt.height;
                    ReleaseEncoder();
                    if (!InitEncoder()) {
                        ReleaseEncoder();
                        return;
                    }
                }

                // 拷贝图像数据
                for (int i = 0; i < 3; ++i) {
                    memcpy(x264_picture_->img.plane[i], frame->data[i], frame->data_len[i]);
                }
                // 编码
                std::shared_ptr<MediaFrame> out_frame;
                if (!Encode(frame, out_frame)) {
                    continue;
                }

                if (out_pin_) {
                    out_pin_->PushMediaFrame(out_frame);
                }

            }
            ReleaseEncoder();
        });
        return true;


    }

    void X264EncoderFilter::Stop() {

    }

    void X264EncoderFilter::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        std::unique_lock<std::mutex> auto_lock(frame_queue_mtx_);
        frame_queue_.push(frame);
        cond_var_.notify_one();
    }

    static void LogX264(void *, int level, const char *format, va_list args) {
        char buf[1024];
        va_list args2;
        va_copy(args2, args);
        int len = vsnprintf(buf, sizeof(buf), format, args2);
        if (len > 0) {
            buf[len - 1] = 0;
        }
                va_end(args2);

        RTC_LOG(LS_INFO) << "x264 log, level: " << level << ", msg: " << buf;
    }

    bool X264EncoderFilter::InitEncoder() {
    x264_param_ = new x264_param_t();
    memset(x264_param_, 0, sizeof(x264_param_t));
    // 设置速率和场景
    if (x264_param_default_preset(x264_param_, encoder_param_.preset.c_str(),
        encoder_param_.tune.c_str()))
    {
        return false;
    }

    // 设置图像的宽高
    x264_param_->i_width = encoder_param_.width;
    x264_param_->i_height = encoder_param_.height;
    // 设置帧率
    x264_param_->i_fps_num = encoder_param_.fps;
    x264_param_->i_fps_den = 1;
    // 设置GOP
    if (encoder_param_.gop > 0) {
        x264_param_->i_keyint_max = encoder_param_.gop;
    }
    // 需要图像的格式
    x264_param_->i_csp = X264_CSP_I420;
    // 不使用B帧, B帧会增大延迟
    x264_param_->i_bframe = 0;
    // 设置单Slice
    x264_param_->i_slice_count = 1;
    // 使用单线程
    x264_param_->i_threads = 1;
    // 每个I帧前面都携带SPS PPS
    x264_param_->b_repeat_headers = 1;

    // 设置码率控制参数
    if ("ABR" == encoder_param_.rate_control) {
        x264_param_->rc.i_rc_method = X264_RC_ABR;
        x264_param_->rc.f_rf_constant = 0.0;
        x264_param_->rc.i_bitrate = encoder_param_.bitrate;
    }
    else {
        x264_param_->rc.i_rc_method = X264_RC_CRF;
        x264_param_->rc.f_rf_constant = (float)encoder_param_.cf;
        x264_param_->rc.i_bitrate = 0;
    }

    // 设置最大码率
    if (encoder_param_.max_bitrate > 0) {
        x264_param_->rc.i_vbv_max_bitrate = encoder_param_.max_bitrate;
    }

    // 设置码率控制缓冲区
    if (encoder_param_.buffer_size > 0) {
        x264_param_->rc.i_vbv_buffer_size = encoder_param_.buffer_size;
    }

    // 根据fps来计算两帧间隔，如果是1，表示使用时间戳来计算间隔
    x264_param_->b_vfr_input = 0;

    // 设置log参数
    x264_param_->pf_log = LogX264;
    x264_param_->p_log_private = nullptr;
    x264_param_->i_log_level = X264_LOG_DEBUG;

    // 设置profile
    if (x264_param_apply_profile(x264_param_, encoder_param_.profile.c_str())) {
        return false;
    }

    // 打开编码器
    x264_ = x264_encoder_open(x264_param_);
    if (!x264_) {
        RTC_LOG(LS_WARNING) << "x264_encoder_open failed";
        return false;
    }

    // 创建pictrue
    x264_picture_ = new x264_picture_t();
    memset(x264_picture_, 0, sizeof(x264_picture_t));
    // 给picture分配空间
    int res = x264_picture_alloc(x264_picture_, x264_param_->i_csp,
        x264_param_->i_width, x264_param_->i_height);
    if (res < 0) {
        RTC_LOG(LS_WARNING) << "x264_picture_alloc failed";
        return false;
    }

    // 设置编码帧的类型
    x264_picture_->i_type = X264_TYPE_AUTO;

    return true;
}


    void X264EncoderFilter::ReleaseEncoder() {
        if (x264_param_) {
            delete x264_param_;
            x264_param_ = nullptr;
        }
        if (x264_picture_) {
            x264_picture_clean(x264_picture_);
            delete x264_picture_;
            x264_picture_ = nullptr;
        }
        if (x264_) {
            x264_encoder_close(x264_);
            x264_ = nullptr;
        }
    }

    bool X264EncoderFilter::Encode(std::shared_ptr<MediaFrame> frame, std::shared_ptr<MediaFrame> &out_frame) {
        // 设置时间戳
        x264_picture_->i_pts = frame->ts;

        int nal_num;
        x264_nal_t* nal_out;
        x264_picture_t pic_out;
        int size = x264_encoder_encode(x264_, &nal_out, &nal_num, x264_picture_, &pic_out);
        if (size < 0) {
            RTC_LOG(LS_WARNING) << "x264_encoder_encode failed: " << size;
            return false;
        }

        if (size == 0) {
            return true;
        }

        int data_size = 0;
        std::vector<x264_nal_t> nals;
        bool idr = false;
        int sps_length = 0;
        char sps[1024] = { 0 };
        int pps_length = 0;
        char pps[1024] = { 0 };

        for (int i = 0; i < nal_num; ++i) {
            x264_nal_t& nal = nal_out[i];
            int nal_type = nal.i_type;
            // 如果NALU是I帧或者P帧
            if (nal_type == NAL_SLICE_IDR || nal_type == NAL_SLICE) {
                nals.push_back(nal);
                data_size += nal.i_payload;

                if (nal_type == NAL_SLICE_IDR) {
                    idr = true;
                }

            }
            else if (nal_type == NAL_SEI) {
                continue;
            }
            else if (nal_type == NAL_SPS) {
                sps_length = nal.i_payload;
                memcpy(sps, nal.p_payload, sps_length);
            }
            else if (nal_type == NAL_PPS) {
                pps_length = nal.i_payload;
                memcpy(pps, nal.p_payload, pps_length);
            }
        }

        if (idr) {
            data_size += (sps_length + pps_length);
        }

        out_frame = std::make_shared<MediaFrame>(data_size);
        out_frame->fmt.media_type = MainMediaType::kMainTypeVideo;
        out_frame->fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeH264;
        out_frame->fmt.sub_fmt.video_fmt.idr = idr;
        out_frame->ts = pic_out.i_pts;
        out_frame->data_len[0] = data_size;
//        out_frame->capture_time_ms = frame->capture_time_ms;

        int data_index = 0;
        for (size_t i = 0; i < nals.size(); ++i) {
            x264_nal_t& nal = nals[i];
            if (nal.i_type == NAL_SLICE_IDR) {
                // 首先拷贝SPS PPS数据
                memcpy(out_frame->data[0] + data_index, sps, sps_length);
                data_index += sps_length;
                memcpy(out_frame->data[0] + data_index, pps, pps_length);
                data_index += pps_length;
                // 拷贝I帧数据
                memcpy(out_frame->data[0] + data_index, nal.p_payload, nal.i_payload);
                data_index += nal.i_payload;
            }
            else {
                // 拷贝P帧数据
                memcpy(out_frame->data[0] + data_index, nal.p_payload, nal.i_payload);
                data_index += nal.i_payload;
            }
        }

        return true;
    }
}

