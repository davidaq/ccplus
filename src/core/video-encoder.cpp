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

struct CCPlus::EncodeContext {
    AVOutputFormat *fmt = 0;
    AVFormatContext *ctx = 0;
    AVStream *audio_st, *video_st = 0;
    AVCodec *audio_codec, *video_codec = 0;

    AVFrame *frame = 0;
    AVPicture srcPic, destPic;
    SwsContext *sws = 0;

    AVFrame *audioFrame = 0;

};

VideoEncoder::VideoEncoder(const std::string& _outputPath, int _fps) :
    outputPath(_outputPath), fps(_fps)
{
}

VideoEncoder::~VideoEncoder() {
    finish();
}

void VideoEncoder::appendFrame(const Frame& frame) {
    if(!ctx) {
        width = frame.getWidth();
        height = frame.getHeight();
        if(width & 1 || height & 1) {
            fprintf(stderr, "The frames should have widths and heights of divisible by 2\n");
            return;
        }
        initContext();
    }
    if(width != frame.getWidth() || height != frame.getHeight()) {
        fprintf(stderr, "Frames encoded into one video should have the same metrics\n");
        return;
    }
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
            codecCtx->sample_fmt  = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
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
            codecCtx->channels      = 1;
            break;
        default:
            break;
    }
    if (ctx->ctx->oformat->flags & AVFMT_GLOBALHEADER)
        codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    return stream;
}

void VideoEncoder::initContext() {
    if(ctx)
        return;
    if(width == 0 || height == 0)
        return;
    ctx = new EncodeContext;
    av_register_all();
    avformat_alloc_output_context2(&(ctx->ctx), NULL, NULL, "something.mp4");
    if(!ctx->ctx) {
        fprintf(stderr, "Can't initialize encoding context\n");
    }
    ctx->fmt = ctx->ctx->oformat;
    ctx->fmt->video_codec = AV_CODEC_ID_MPEG4;
    ctx->fmt->audio_codec = AV_CODEC_ID_AAC;
    ctx->video_st = initStream(ctx->video_codec, AV_CODEC_ID_MPEG4);
    ctx->audio_st = initStream(ctx->audio_codec, AV_CODEC_ID_AAC);

    // init video codec
    AVCodecContext* c = ctx->video_st->codec;
    if(0 < avcodec_open2(c, ctx->video_codec, NULL)) {
        fprintf(stderr, "Could not open video encoder\n");
        return;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return;
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;
    ctx->frame = frame;

    if(0 < avpicture_alloc(&(ctx->destPic), AV_PIX_FMT_YUV420P, width, height)) {
        fprintf(stderr, "Could not allocate yuv420p picture buffer\n");
        return;
    }
    if(0 < avpicture_alloc(&(ctx->srcPic), AV_PIX_FMT_BGRA, width, height)) {
        fprintf(stderr, "Could not allocate bgra picture buffer\n");
        return;
    }
    *((AVPicture*)frame) = ctx->destPic;

    // init sws context
    ctx->sws = sws_getContext(width, height, AV_PIX_FMT_BGRA, width, height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(!ctx->sws) {
        fprintf(stderr, "Could not init image resampler\n");
        return;
    }

    // init audio codec
    c = ctx->audio_st->codec;
    AVDictionary *opts = 0;
    av_dict_set(&opts, "strict", "experimental", 0);
    if(0 < avcodec_open2(c, ctx->audio_codec, &opts)) {
        fprintf(stderr, "Could not open audio encoder\n");
        return;
    }
    av_dict_free(&opts);
    ctx->audioFrame = av_frame_alloc();
    if(!ctx->audioFrame) {
        fprintf(stderr, "Could not alloc audio frame\n");
        return;
    }
}

void VideoEncoder::releaseContext() {
    if(!ctx)
        return;
    if(ctx->video_st) {
        avcodec_close(ctx->video_st->codec);
        av_free(ctx->srcPic.data[0]);
        av_free(ctx->destPic.data[0]);
        av_frame_free(&(ctx->frame));
        sws_freeContext(ctx->sws);
    }
    if(ctx->audio_st) {
        avcodec_close(ctx->audio_st->codec);
        av_frame_free(&(ctx->audioFrame));
    }
    if(ctx->ctx)
        avformat_free_context(ctx->ctx);
    delete ctx;
    ctx = 0;
}

