#define __STDC_CONSTANT_MACROS
#define __FFMPEG__
extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include "video-encoder.hpp"
#include "global.hpp"

using namespace CCPlus;

struct CCPlus::DecodeContext {
    AVOutputFormat *fmt;
    AVFormatContext *ctx;
    AVStream *audio_st, *video_st;
    AVCodec *audio_codec, *video_codec;
};

VideoEncoder::VideoEncoder(const std::string& _outputPath, int _fps) :
    outputPath(_outputPath), fps(_fps)
{
}

VideoEncoder::~VideoEncoder() {
    finish();
}

void VideoEncoder::appendImage(const Image& frame) {

}

void VideoEncoder::finish() {
    if(!ctx)
        return;
}

AVStream* VideoEncoder::initStream(AVCodec*& codec, enum AVCodecID codec_id) {
    AVStream* stream;

    codec = avcodec_find_encoder(codec_id);
    if(!codec) {
        fprintf(stderr, "Encoder not found\n");
        return 0;
    }
    stream = avformat_new_stream(ctx->ctx, codec);
    if(!stream) {
        fprintf(stderr, "Can't alloc stream\n");
        return 0;
    }
    stream->id = ctx->ctx->nb_streams - 1;
    AVCodecContext* codecCtx = stream->codec;
    switch(codec->type) {
        case AVMEDIA_TYPE_AUDIO:
            codecCtx->sample_fmt  = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            codecCtx->bit_rate    = 64000;
            codecCtx->sample_rate = 24000;
            codecCtx->channels    = 1;
            break;
        case AVMEDIA_TYPE_VIDEO:
            codecCtx->codec_id = codec_id;
            codecCtx->bit_rate = 400000;
            codecCtx->width    = width;
            codecCtx->height   = height;
            codecCtx->time_base.den = fps;
            codecCtx->time_base.num = 1;
            codecCtx->gop_size      = 12; 
            codecCtx->pix_fmt       = AV_PIX_FMT_YUV420P;
            break;
        default:
            break;
    }
    return stream;
}

void VideoEncoder::initContext() {
    if(ctx)
        return;
    if(width == 0 || height == 0)
        return;
    ctx = new DecodeContext;
    av_register_all();
    avformat_alloc_output_context2(&(ctx->ctx), NULL, NULL, "something.mp4");
    if(!ctx->ctx) {
        fprintf(stderr, "Can't initialize encoding context\n");
    }
    ctx->fmt = ctx->ctx->oformat;
    ctx->fmt->video_codec = AV_CODEC_ID_H264;
    ctx->fmt->audio_codec = AV_CODEC_ID_AAC;
}

void VideoEncoder::releaseContext() {
    if(!ctx)
        return;
}

