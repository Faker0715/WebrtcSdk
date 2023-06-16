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
        }
        running_ = true;
        encoder_thread_ = new std::thread([=]() {
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
                    std::unique_lock<std::mutex> auto_lock(frame_queue_mutex_);
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

            }
        });
        return true;


    }

    void X264EncoderFilter::Stop() {

    }

    void X264EncoderFilter::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        std::unique_lock<std::mutex> auto_lock(frame_queue_mutex_);
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
                                      encoder_param_.tune.c_str())) {
            return false;
        }
        // 设置图像的宽高
        x264_param_->i_width = encoder_param_.width;
        x264_param_->i_height = encoder_param_.height;
        // 设置帧率
        x264_param_->i_fps_num = encoder_param_.fps;
        x264_param_->i_fps_den = 1;
        // GOP
        if (encoder_param_.gop > 0) {
            // 60帧产生一个关键帧
            x264_param_->i_keyint_max = encoder_param_.gop;
        }
        // 图像格式
        x264_param_->i_csp = X264_CSP_I420;
        // 不使用B帧
        x264_param_->i_bframe = 0;
        // 设置单slice
        x264_param_->i_slice_count = 1;
        // 使用单线程
        x264_param_->i_threads = 1;
        // 每个i帧前面都携带sps pps
        x264_param_->b_repeat_headers = 1;

        // 设置码率控制参数
        if ("ABR" == encoder_param_.rate_control) {
            x264_param_->rc.i_rc_method = X264_RC_ABR;
            x264_param_->rc.f_rf_constant = 0.0;
            x264_param_->rc.i_bitrate = encoder_param_.bitrate;
        } else {
            x264_param_->rc.i_rc_method = X264_RC_CRF;
            x264_param_->rc.f_rf_constant = (float) encoder_param_.cf;
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


        return true;
    }

    void X264EncoderFilter::ReleaseEncoder() {
        if (x264_param_) {
            delete x264_param_;
            x264_param_ = nullptr;
        }
    }
}