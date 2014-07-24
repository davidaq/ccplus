#include "video-decoder.hpp"
#include "global.hpp"
#include "utils.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define __STDC_FORMAT_MACROS
extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}
using namespace CCPlus;

struct DecodeContext {
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    const char *src_filename = NULL;
    const char *video_dst_filename = NULL;
    const char *audio_dst_filename = NULL;
    
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

#define CTX (*((DecodeContext*)this->decodeContext))

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
    return CTX.info;
}

void VideoDecoder::seekTo(float time) {
    if (decoderFlag & DECODE_VIDEO) {
        initContext();
        cursorTime = time;
        time -= 1;
        if(time < 1) {
            time = 0;
        }
        av_seek_frame(CTX.fmt_ctx, -1, time * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    }
}

bool VideoDecoder::readNextFrameIfNeeded() {
    initContext();
    if(CTX.haveCurrentPkt)
        return true;
    if(av_read_frame(CTX.fmt_ctx, &(CTX.pkt)) >= 0) {
        CTX.currentPkt = CTX.pkt;
        CTX.haveCurrentPkt = true;
        return true;
    }
    return false;
}

float VideoDecoder::decodeImage() {
    haveDecodedImage = false;
    decodedImage = 0;
    float retTime = -1;
    while(!haveDecodedImage) {
        if(!readNextFrameIfNeeded())
            return -1;
        if (CTX.pkt.stream_index == CTX.video_stream_idx) {
            int gotFrame = 0;
            int ret = avcodec_decode_video2(CTX.video_dec_ctx, CTX.frame, &gotFrame, &(CTX.pkt));
            if(ret < 0)
                return -1;
            if(gotFrame) {
                retTime = av_rescale_q(av_frame_get_best_effort_timestamp(CTX.frame), 
                                        CTX.fmt_ctx->streams[CTX.pkt.stream_index]->time_base, 
                                        AV_TIME_BASE_Q)
                            * 0.000001;
                if(retTime + 0.01 >= cursorTime) {
                    haveDecodedImage = true;
                }
            }
            CTX.pkt.data += ret;
            CTX.pkt.size -= ret;
            
            if(CTX.pkt.size <= 0) {
                av_free_packet(&(CTX.currentPkt));
                CTX.haveCurrentPkt = false;
            }
        } else {
            av_free_packet(&(CTX.currentPkt));
            CTX.haveCurrentPkt = false;
        }
    }
    cursorTime = retTime;
    return retTime;
}

Image VideoDecoder::getDecodedImage() {
    if(!haveDecodedImage)
        return Image();
    else if(!decodedImage) {
        if(!CTX.swsContext) {
            CTX.swsContext = sws_getContext(CTX.info.width, CTX.info.height, 
                                            CTX.video_dec_ctx->pix_fmt,
                                            CTX.info.width, CTX.info.height,
                                            PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
            CTX.imagebuff.linesize[0] = CTX.info.width * 4;
            CTX.imagebuff.data[0] = (uint8_t*)malloc(CTX.imagebuff.linesize[0] * CTX.info.height);
        }
        sws_scale(CTX.swsContext, CTX.frame->data, CTX.frame->linesize, 0, CTX.info.height, CTX.imagebuff.data, CTX.imagebuff.linesize);
        cv::Mat data = cv::Mat(CTX.info.height, CTX.info.width, CV_8UC4, CTX.imagebuff.data[0]);
        if(CTX.rotate) {
            switch(CTX.rotate) {
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
        decodedImage = new Image(data);
    }
    return *decodedImage;
}

int VideoDecoder::decodeAudioFrame(FILE* destFile, float duration, float &start, float &gap) {
    int gotFrame = 0;
    int ret = avcodec_decode_audio4(CTX.audio_dec_ctx, CTX.frame, &gotFrame, &(CTX.pkt));
    if(ret < 0)
        return -1;
    if(gotFrame) {
        float retTime = av_rescale_q(av_frame_get_best_effort_timestamp(CTX.frame), 
                                CTX.fmt_ctx->streams[CTX.pkt.stream_index]->time_base, 
                                AV_TIME_BASE_Q)
                    * 0.000001;
        if(retTime + 0.01 >= cursorTime) {
            cursorTime = retTime;
            if(gap < 0)
                gap = (cursorTime - start) / 3;
            AVCodecContext *dec = CTX.audio_stream->codec;
            if(!CTX.swrContext) {
                CTX.swrContext = swr_alloc();

                av_opt_set_int(CTX.swrContext, "in_channel_layout", dec->channel_layout, 0);
                av_opt_set_int(CTX.swrContext, "in_sample_rate", dec->sample_rate, 0);
                av_opt_set_sample_fmt(CTX.swrContext, "in_sample_fmt", dec->sample_fmt, 0);

                av_opt_set_int(CTX.swrContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
                av_opt_set_int(CTX.swrContext, "out_sample_rate", 24000, 0);
                av_opt_set_sample_fmt(CTX.swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

                if(swr_init(CTX.swrContext) < 0) {
                    fprintf(stderr, "Can't init SWResample context\n");
                    return -1;
                }
            }
            int dst_nb_samples = av_rescale_rnd(0
                    + CTX.frame->nb_samples, 24000, dec->sample_rate, AV_ROUND_UP);
            if(dst_nb_samples > CTX.swrDestSamples) {
                CTX.swrDestSamples = dst_nb_samples;
                int dst_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO);
                if(CTX.swrDestBuffer)
                    av_free(CTX.swrDestBuffer[0]);
                int res = av_samples_alloc_array_and_samples(
                        &(CTX.swrDestBuffer), &(CTX.swrLinesize), dst_nb_channels, dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
                if(res < 0) {
                    fprintf(stderr, "Failed to alloc memory\n");
                    return -1;
                }
            }
            size_t unpadded_linesize = dst_nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            swr_convert(CTX.swrContext, CTX.swrDestBuffer, unpadded_linesize,
                    (const uint8_t**)(CTX.frame->extended_data), CTX.frame->nb_samples);
            fwrite(CTX.swrDestBuffer[0], 1, unpadded_linesize, destFile);
            if(duration > 0 && cursorTime - start - gap > duration){
                return -5;
            }
        }
    }
    return ret;
}

void VideoDecoder::decodeAudio(FILE* destFile, float duration) {
    float start = cursorTime;
    float gap = -1;
    bool goon = true;
    while(goon && readNextFrameIfNeeded()) {
        if (CTX.pkt.stream_index == CTX.audio_stream_idx) {
            do {
                int ret = decodeAudioFrame(destFile, duration, start, gap);
                goon = ret > 0;
                CTX.pkt.data += ret;
                CTX.pkt.size -= ret;
            } while(goon && CTX.pkt.size > 0);
        }
        av_free_packet(&(CTX.currentPkt));
        CTX.haveCurrentPkt = false;
    }
}

void VideoDecoder::decodeAudio(const std::string& outputFile, float duration) {
    if(!(decoderFlag & DECODE_AUDIO)) {
        return;
    }
    initContext();
    FILE* destFile = fopen(outputFile.c_str(), "wb");
    if(!destFile)
        return;
    decodeAudio(destFile, duration);
    fclose(destFile);
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
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
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
    }
    if(avformat_open_input(&(CTX.fmt_ctx), inputFile.c_str(), NULL, NULL) < 0) {
        releaseContext();
        return;
    }
    if (avformat_find_stream_info(CTX.fmt_ctx, NULL) < 0) {
        releaseContext();
        return;
    }
    CTX.info = VideoInfo {
        .duration = static_cast<float>(CTX.fmt_ctx->duration * 1.0 / AV_TIME_BASE),
        .width = 0,
        .height = 0
    };
    if (decoderFlag & DECODE_VIDEO && open_codec_context(&(CTX.video_stream_idx), CTX.fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        CTX.video_stream = CTX.fmt_ctx->streams[CTX.video_stream_idx];
        CTX.video_dec_ctx = CTX.video_stream->codec;

        CTX.info.width = CTX.video_dec_ctx->width;
        CTX.info.height = CTX.video_dec_ctx->height;
        
        
        AVDictionaryEntry *tag = NULL;
        tag = av_dict_get(CTX.video_stream->metadata, "rotate", tag, AV_DICT_MATCH_CASE);
        if(tag) {
            CTX.rotate = parseString<int>(tag->value);
        }
    }
    if (decoderFlag & DECODE_AUDIO && open_codec_context(&(CTX.audio_stream_idx), CTX.fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        CTX.audio_stream = CTX.fmt_ctx->streams[CTX.audio_stream_idx];
        CTX.audio_dec_ctx = CTX.audio_stream->codec;
    }
    if (!CTX.audio_stream && !CTX.video_stream) {
        releaseContext();
        return;
    }
    cursorTime = 0;
    CTX.frame = av_frame_alloc();
    if (!CTX.frame) {
        releaseContext();
        return;
    }

    av_init_packet(&(CTX.pkt));
    CTX.pkt.data = NULL;
    CTX.pkt.size = 0;
}

void VideoDecoder::releaseContext() {
    if(!decodeContext)
        return;
    if(CTX.video_dec_ctx)
        avcodec_close(CTX.video_dec_ctx);
    if(CTX.audio_dec_ctx)
        avcodec_close(CTX.audio_dec_ctx);
    if(CTX.fmt_ctx)
        avformat_close_input(&(CTX.fmt_ctx));
    if(CTX.frame)
        av_frame_free(&CTX.frame);
    if(CTX.swsContext) {
        sws_freeContext(CTX.swsContext);
        free(CTX.imagebuff.data[0]);
        CTX.swsContext = 0;
    }
    if(CTX.swrContext) {
        swr_free(&CTX.swrContext);
        CTX.swrContext = 0;
    }
    if(CTX.swrDestBuffer) {
        av_free(CTX.swrDestBuffer[0]);
        CTX.swrDestBuffer = 0;
        CTX.swrDestSamples = 0;
    }
    delete ((DecodeContext*)decodeContext);
    decodeContext = 0;
    cursorTime = -1;
}

