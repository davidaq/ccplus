#define VIDEO_DECODER
#include "video-decoder.hpp"
#include "global.hpp"
#include "utils.hpp"
#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS
extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

struct CCPlus::DecodeContext {
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    
    int video_stream_idx = -1, audio_stream_idx = -1;
    AVFrame *frame = NULL;
    AVPacket pkt, currentPkt;
    int haveCurrentPkt = 0;
    int video_frame_count = 0;
    int audio_frame_count = 0;
    int rotate = 0;
    VideoInfo info;
    
    SwsContext *swsContext = 0;
    AVPicture imagebuff;

    SwrContext *swrContext = 0;
    int swrLinesize = 0;
    int swrDestSamples = 0;
    uint8_t** swrDestBuffer = 0;
};

using namespace CCPlus;

VideoDecoder::VideoDecoder(const std::string& _inputFile, int _decoderFlag) :
    inputFile(_inputFile),
    decoderFlag(_decoderFlag),
    cursorTime(-1),
    decodeContext(0)
{
}

VideoDecoder::~VideoDecoder() {
    releaseContext();
}

VideoInfo VideoDecoder::getVideoInfo() {
    initContext();
    if(invalid) return VideoInfo();
    return decodeContext->info;
}

void VideoDecoder::seekTo(float time) {
    initContext();
    if(invalid) return;
    cursorTime = time;
    time -= 1;
    if(time < 1) {
        time = 0;
    }
    av_seek_frame(decodeContext->fmt_ctx, -1, time * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
}

bool VideoDecoder::readNextFrameIfNeeded() {
    initContext();
    if(invalid) return false;
    if(decodeContext->haveCurrentPkt)
        return true;
    int avRet = av_read_frame(decodeContext->fmt_ctx, &(decodeContext->pkt));
    if(avRet == AVERROR(EAGAIN)) {
        usleep(10000);
        return readNextFrameIfNeeded();
    } else if(avRet < 0) {
        return false;
    }
    decodeContext->currentPkt = decodeContext->pkt;
    decodeContext->haveCurrentPkt = true;
    return true;
}

float VideoDecoder::decodeImage() {
    initContext();
    if(!(decoderFlag & DECODE_VIDEO))
        return -1;
    haveDecodedImage = false;
    if(decodedImage)
        delete decodedImage;
    decodedImage = 0;
    float retTime = -1;
    int brutal = 10;
    while(!haveDecodedImage && brutal > 0) {
        if(!readNextFrameIfNeeded())
            brutal--;
        if (decodeContext->pkt.stream_index == decodeContext->video_stream_idx) {
            int gotFrame = 0;
            int ret = avcodec_decode_video2(decodeContext->video_dec_ctx, decodeContext->frame, &gotFrame, &(decodeContext->pkt));
            if(ret < 0)
                return -1;
            if(gotFrame) {
                retTime = av_rescale_q(av_frame_get_best_effort_timestamp(decodeContext->frame), 
                                        decodeContext->fmt_ctx->streams[decodeContext->pkt.stream_index]->time_base, 
                                        AV_TIME_BASE_Q)
                            * 0.000001;
                if(retTime + 0.01 >= cursorTime) {
                    haveDecodedImage = true;
                }
            }
            decodeContext->pkt.data += ret;
            decodeContext->pkt.size -= ret;
            
            if(decodeContext->pkt.size <= 0) {
                av_free_packet(&(decodeContext->currentPkt));
                decodeContext->haveCurrentPkt = false;
            }
        } else {
            av_free_packet(&(decodeContext->currentPkt));
            decodeContext->haveCurrentPkt = false;
        }
    }
    if(brutal <= 0) {
        return -1;
    }
    cursorTime = retTime;
    return retTime;
}

Frame VideoDecoder::getDecodedImage() {
    if(!haveDecodedImage) {
        return Frame();
    } else if(!decodedImage) {
        if(!decodeContext->swsContext) {
            decodeContext->swsContext = sws_getContext(decodeContext->info.width, decodeContext->info.height, 
                                            decodeContext->video_dec_ctx->pix_fmt,
                                            decodeContext->info.width, decodeContext->info.height,
                                            PIX_FMT_BGRA, SWS_POINT, NULL, NULL, NULL);
            decodeContext->imagebuff.linesize[0] = decodeContext->info.width * 4;
            decodeContext->imagebuff.data[0] = (uint8_t*)malloc(decodeContext->imagebuff.linesize[0] * decodeContext->info.height);
        }
        sws_scale(decodeContext->swsContext, decodeContext->frame->data, decodeContext->frame->linesize, 0, decodeContext->info.height, decodeContext->imagebuff.data, decodeContext->imagebuff.linesize);
        cv::Mat data = cv::Mat(decodeContext->info.height, decodeContext->info.width, CV_8UC4);
        memcpy(data.data, decodeContext->imagebuff.data[0], decodeContext->imagebuff.linesize[0] * decodeContext->info.height);
        if(decodeContext->rotate) {
            switch(decodeContext->rotate) {
            case 180:
                flip(data, data, -1); 
                break;
            case 90:
                transpose(data, data);
                flip(data, data, 1); 
                break;
            case 270:
                transpose(data, data);
                flip(data, data, 0); 
            }
        }
        decodedImage = new Frame();
        decodedImage->image = data;
    }
    return *decodedImage;
}

int VideoDecoder::decodeAudioFrame(std::function<void(const void*, size_t, size_t)> output, float& retTime) {
    int gotFrame = 0;
    int ret = avcodec_decode_audio4(decodeContext->audio_dec_ctx, decodeContext->frame, &gotFrame, &(decodeContext->pkt));
    if(ret < 0)
        return -1;
    if(gotFrame) {
        retTime = av_rescale_q(
            av_frame_get_best_effort_timestamp(decodeContext->frame), 
            decodeContext->fmt_ctx->streams[decodeContext->pkt.stream_index]->time_base, 
            AV_TIME_BASE_Q) * 0.000001;
        if(retTime + 0.5 >= cursorTime) {
            cursorTime = retTime;
            AVCodecContext *dec = decodeContext->audio_stream->codec;
            if(!decodeContext->swrContext) {
                decodeContext->swrContext = swr_alloc();

                av_opt_set_int(decodeContext->swrContext, "in_channel_layout", dec->channel_layout, 0);
                av_opt_set_int(decodeContext->swrContext, "in_sample_rate", dec->sample_rate, 0);
                av_opt_set_sample_fmt(decodeContext->swrContext, "in_sample_fmt", dec->sample_fmt, 0);

                av_opt_set_int(decodeContext->swrContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
                av_opt_set_int(decodeContext->swrContext, "out_sample_rate", CCPlus::audioSampleRate, 0);
                av_opt_set_sample_fmt(decodeContext->swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

                if(swr_init(decodeContext->swrContext) < 0) {
                    log(logERROR) << "Can't init SWResample context";
                    return -1;
                }
            }
            int dst_nb_samples = av_rescale_rnd(0
                    + decodeContext->frame->nb_samples, CCPlus::audioSampleRate, dec->sample_rate, AV_ROUND_UP);
            if(dst_nb_samples > decodeContext->swrDestSamples) {
                decodeContext->swrDestSamples = dst_nb_samples;
                int dst_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO);
                if(decodeContext->swrDestBuffer)
                    av_free(decodeContext->swrDestBuffer[0]);
                int res = av_samples_alloc_array_and_samples(
                        &(decodeContext->swrDestBuffer), &(decodeContext->swrLinesize), dst_nb_channels, dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
                if(res < 0) {
                    log(logERROR) << "Failed to alloc memory";
                    return -1;
                }
            }
            size_t unpadded_linesize = dst_nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            swr_convert(decodeContext->swrContext, decodeContext->swrDestBuffer, unpadded_linesize,
                    (const uint8_t**)(decodeContext->frame->extended_data), decodeContext->frame->nb_samples);
            output(decodeContext->swrDestBuffer[0], 1, unpadded_linesize);
        } else {
            retTime = -1;
        }
    }
    return ret;
}

float VideoDecoder::decodeAudio(std::function<void(const void*, size_t, size_t)> output, float duration) {
    float start = cursorTime - 0.5;
    bool goon = true;
    float realStart = -1;
    float ctime;
    while(goon && readNextFrameIfNeeded()) {
        if (decodeContext->pkt.stream_index == decodeContext->audio_stream_idx) {
            do {
                int ret = decodeAudioFrame(output, ctime);
                if(ctime >= 0)
                    realStart = ctime;
                if(ret < 0)
                    goon = false;
                decodeContext->pkt.data += ret;
                decodeContext->pkt.size -= ret;
            } while(goon && decodeContext->pkt.size > 0);
        }
        if(ctime - start > duration)
            goon = false;
        av_free_packet(&(decodeContext->currentPkt));
        decodeContext->haveCurrentPkt = false;
    }
    return realStart;
}

void VideoDecoder::decodeAudio(const std::string& outputFile, float duration) {
    if(!(decoderFlag & DECODE_AUDIO)) {
        return;
    }
    initContext();
    if(invalid) return;
    FILE* destFile = fopen(outputFile.c_str(), "wb");
    if(!destFile)
        return;
    decodeAudio(destFile, duration);
    fclose(destFile);
}

void VideoDecoder::decodeAudio(FILE* destFile, float duration) {
    auto output = [&destFile] (const void* buffer, size_t size, size_t count) {
        fwrite(buffer, size, count, destFile);    
    };
    decodeAudio(output, duration);
}

std::vector<int16_t> VideoDecoder::decodeAudio(float duration) {
    std::vector<int16_t> ret;
    float cTime = cursorTime;
    float realStart = decodeAudio(ret, duration);
    cTime -= realStart;
    if(cTime > 0 && !ret.empty()) {
        int s = (int)(cTime * CCPlus::audioSampleRate);
        if(s > 0) {
            ret = std::vector<int16_t>(ret.begin() + s, ret.end());
        }
    }
    return ret;
}

float VideoDecoder::decodeAudio(std::vector<int16_t>& ret, float duration) {
    auto output = [&ret] (const void* buffer, size_t size, size_t count) {
        // 2 bytes per int16_t
        for (int i = 0; i < count / 2; i++) {
            ret.push_back(((int16_t*) buffer)[i]);
        }
    };
    return decodeAudio(output, duration);
}

static int open_codec_context(int *stream_idx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        return ret;
    } else {
        *stream_idx = ret;
        st = fmt_ctx->streams[*stream_idx];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            log(logERROR) << "Failed to find codec " << av_get_media_type_string(type);
            return AVERROR(EINVAL);
        }

        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            log(logERROR) << "Failed to open codec " << av_get_media_type_string(type);
            return ret;
        }
    }

    return 0;
}

void VideoDecoder::initContext() {
    if(decodeContext)
        return;
    decodeContext = new DecodeContext;
    static bool avregistered = false;
    if(!avregistered) {
        avregistered = true;
        av_register_all();
        sleep(1);
    }
    if(avformat_open_input(&(decodeContext->fmt_ctx), inputFile.c_str(), NULL, NULL) < 0) {
        log(logERROR) << "Faild to open decode context for: " << inputFile;
        releaseContext();
        return;
    }
    if (avformat_find_stream_info(decodeContext->fmt_ctx, NULL) < 0) {
        releaseContext();
        return;
    }
    decodeContext->info = VideoInfo {
        .duration = static_cast<float>(decodeContext->fmt_ctx->duration * 1.0 / AV_TIME_BASE),
        .width = 0,
        .height = 0,
        .rwidth = 0,
        .rheight = 0
    };
    if (decoderFlag & DECODE_VIDEO && 
            open_codec_context(&(decodeContext->video_stream_idx), decodeContext->fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        decodeContext->video_stream = decodeContext->fmt_ctx->streams[decodeContext->video_stream_idx];
        decodeContext->video_dec_ctx = decodeContext->video_stream->codec;

        decodeContext->info.width = decodeContext->video_dec_ctx->width;
        decodeContext->info.rwidth = decodeContext->video_dec_ctx->width;
        decodeContext->info.height = decodeContext->video_dec_ctx->height;
        decodeContext->info.rheight = decodeContext->video_dec_ctx->height;
        
        AVDictionaryEntry *tag = NULL;
        tag = av_dict_get(decodeContext->video_stream->metadata, "rotate", tag, AV_DICT_MATCH_CASE);
        if(tag) {
            decodeContext->rotate = atoi(tag->value);
            if(decodeContext->rotate != 0 && decodeContext->rotate != 180) {
                decodeContext->info.rwidth = decodeContext->info.height;
                decodeContext->info.rheight = decodeContext->info.width;
            }
        }
    }
    if (decoderFlag & DECODE_AUDIO &&
            open_codec_context(&(decodeContext->audio_stream_idx), decodeContext->fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        decodeContext->audio_stream = decodeContext->fmt_ctx->streams[decodeContext->audio_stream_idx];
        decodeContext->audio_dec_ctx = decodeContext->audio_stream->codec;
    }

    if (!decodeContext->audio_stream && !decodeContext->video_stream) {
        releaseContext();
        return;
    }
    cursorTime = 0;
    decodeContext->frame = av_frame_alloc();
    if (!decodeContext->frame) {
        releaseContext();
        return;
    }

    av_init_packet(&(decodeContext->pkt));
    decodeContext->pkt.data = NULL;
    decodeContext->pkt.size = 0;
    invalid = false;

    if(decoderFlag & DECODE_VIDEO && decodeContext->video_stream) {
        decodeImage();
        if(decodeImage() < 0) {
            decoderFlag -= DECODE_VIDEO;
        }
        seekTo(0);
    }
}

void VideoDecoder::releaseContext() {
    invalid = true;
    if(!decodeContext)
        return;
    if(decodeContext->video_dec_ctx)
        avcodec_close(decodeContext->video_dec_ctx);
    if(decodeContext->audio_dec_ctx)
        avcodec_close(decodeContext->audio_dec_ctx);
    if(decodeContext->fmt_ctx)
        avformat_close_input(&(decodeContext->fmt_ctx));
    if(decodeContext->frame)
        av_frame_free(&decodeContext->frame);
    if(decodeContext->swsContext) {
        sws_freeContext(decodeContext->swsContext);
        free(decodeContext->imagebuff.data[0]);
        decodeContext->swsContext = 0;
    }
    if(decodeContext->swrContext) {
        swr_free(&decodeContext->swrContext);
        decodeContext->swrContext = 0;
    }
    if(decodeContext->swrDestBuffer) {
        av_free(decodeContext->swrDestBuffer[0]);
        decodeContext->swrDestBuffer = 0;
        decodeContext->swrDestSamples = 0;
    }
    delete ((DecodeContext*)decodeContext);
    decodeContext = 0;
    cursorTime = -1;
}

