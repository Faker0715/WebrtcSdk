//
// Created by faker on 2023/6/29.
//

#include "xrtc/media/filter/opus_encoder_filter.h"

#include <rtc_base/logging.h>
#include <rtc_base/thread.h>
#include <opus/opus.h>

#include "xrtc/media/base/in_pin.h"
#include "xrtc/media/base/out_pin.h"

namespace xrtc {

    namespace {

        const int kDefaultComplexity = 9;

    } // namespace

    OpusEncoderFilter::OpusEncoderFilter() :
            in_pin_(std::make_unique<InPin>(this)),
            out_pin_(std::make_unique<OutPin>(this)),
            opus_app_(OPUS_APPLICATION_VOIP)
    {
        MediaFormat fmt_in;
        fmt_in.media_type = MainMediaType::kMainTypeAudio;
        fmt_in.sub_fmt.audio_fmt.type = SubMediaType::kSubTypePcm;
        in_pin_->set_format(fmt_in);

        MediaFormat fmt_out;
        fmt_out.media_type = MainMediaType::kMainTypeAudio;
        fmt_out.sub_fmt.audio_fmt.type = SubMediaType::kSubTypeOpus;
        out_pin_->set_format(fmt_out);
    }

    OpusEncoderFilter::~OpusEncoderFilter() {
    }

    bool OpusEncoderFilter::Start() {
        RTC_LOG(LS_INFO) << "OpusEncoderFilter Start";

        if (running_) {
            RTC_LOG(LS_WARNING) << "OpusEncoderFilter already Start, ignore";
            return true;
        }

        running_ = true;

        encoder_thread_ = new std::thread([=]() {
            RTC_LOG(LS_INFO) << "OpusEncoderFilter encode thread start";
            rtc::SetCurrentThreadName("opus_encode_thread");

            // 创建opus编码器
            OpusEncoder* encoder = CreateEncoder();
            if (!encoder) {
                RTC_LOG(LS_WARNING) << "opus CreateEncoder() failed";
                running_ = false;
                return;
            }

            size_t frame_queue_size = 0;
            unsigned char encoded_buffer[1500] = {};

            while (running_) {
                // 从队列当中获取一帧音频数据
                std::shared_ptr<MediaFrame> frame;
                {
                    std::unique_lock<std::mutex> lock(frame_queue_mtx_);
                    frame_queue_size = frame_queue_.size();
                    if (frame_queue_size > 0) {
                        frame = frame_queue_.front();
                        frame_queue_.pop();
                    }

                    if (!frame) {
                        cond_var_.wait(lock);
                        continue;
                    }
                }

                // 获得了一帧音频数据
                // 根据音频数据动态调整编码器
                if (sample_rate_hz_ != frame->fmt.sub_fmt.audio_fmt.samples_per_sec ||
                    channels_ != frame->fmt.sub_fmt.audio_fmt.channels)
                {
                    RTC_LOG(LS_INFO) << "OpusEncoderFilter encode param changed"
                                     << ", sample_rate_hz: " << sample_rate_hz_
                                     << ", dst_sample_rate_hz: " << frame->fmt.sub_fmt.audio_fmt.samples_per_sec
                                     << ", channels: " << channels_
                                     << ", dst_channels: " << frame->fmt.sub_fmt.audio_fmt.channels;
                    if (encoder) {
                        sample_rate_hz_ = frame->fmt.sub_fmt.audio_fmt.samples_per_sec;
                        channels_ = frame->fmt.sub_fmt.audio_fmt.channels;
                        opus_encoder_destroy(encoder);
                        encoder = CreateEncoder();
                    }
                }

                if (!encoder) {
                    RTC_LOG(LS_WARNING) << "opus CreateEncoder() failed";
                    running_ = false;
                    return;
                }

                // 开始编码数据
                int ret = opus_encode(encoder, (const opus_int16*)frame->data[0],
                                      frame->fmt.sub_fmt.audio_fmt.samples_per_channel,
                                      encoded_buffer, sizeof(encoded_buffer));
                if (ret < 0) {
                    RTC_LOG(LS_WARNING) << "opus encode failed: " << ret;
                    running_ = false;
                    opus_encoder_destroy(encoder);
                    return;
                }

                // 创建新的media frame
                auto output_frame = std::make_shared<MediaFrame>(ret);
                output_frame->fmt.media_type = MainMediaType::kMainTypeAudio;
                output_frame->fmt.sub_fmt = frame->fmt.sub_fmt;
                output_frame->fmt.sub_fmt.audio_fmt.type = SubMediaType::kSubTypeOpus;
                output_frame->data_len[0] = ret;
                memcpy(output_frame->data[0], encoded_buffer, ret);
                output_frame->ts = frame->ts;

                if (out_pin_) {
                    out_pin_->PushMediaFrame(output_frame);
                }
            }

            opus_encoder_destroy(encoder);
        });

        return true;
    }

    void OpusEncoderFilter::Setup(const std::string& json_config) {
    }

    void OpusEncoderFilter::Stop() {
        RTC_LOG(LS_INFO) << "OpusEncoderFilter Stop";
        if (!running_) {
            return;
        }

        running_ = false;
        cond_var_.notify_all();

        if (encoder_thread_ && encoder_thread_->joinable()) {
            encoder_thread_->join();
            RTC_LOG(LS_INFO) << "opus encoder thread join success";
            delete encoder_thread_;
            encoder_thread_ = nullptr;
        }
    }

    void OpusEncoderFilter::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
        // 在音频采集线程触发
        std::unique_lock<std::mutex> lock(frame_queue_mtx_);
        frame_queue_.push(frame);
        cond_var_.notify_one();
    }

    OpusEncoder* OpusEncoderFilter::CreateEncoder() {
        int err = 0;
        OpusEncoder* encoder = opus_encoder_create(sample_rate_hz_, channels_,
                                                   opus_app_, &err);
        if (err != OPUS_OK || !encoder) {
            RTC_LOG(LS_WARNING) << "opus_encoder_create failed, err: " << err;
            return nullptr;
        }

        // 设置核心的编码参数
        do {
            // 设置编码码率
            if (opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate_)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_BITRATE failed";
                break;
            }

            // 设置带宽(auto)
            if (opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_AUTO)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_BANDWIDTH failed";
                break;
            }

            // 设置最大带宽
            if (opus_encoder_ctl(encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_MAX_BANDWIDTH failed";
                break;
            }

            // 设置复杂度
            if (opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(kDefaultComplexity)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_MAX_BANDWIDTH failed";
                break;
            }

            // 启用FEC
            if (opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_INBAND_FEC failed";
                break;
            }

            // 设置丢包率
            if (opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(5)) != OPUS_OK) {
                RTC_LOG(LS_WARNING) << "OPUS_SET_PACKET_LOSS_PERC failed";
                break;
            }

            return encoder;

        } while (false);

        opus_encoder_destroy(encoder);
        return nullptr;
    }

} // namespace xrtc
