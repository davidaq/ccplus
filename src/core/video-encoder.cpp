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
    SwrContext *swr = 0;
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
            fprintf(stderr, "The frames must have widths and heights divisible by 2\n");
            return;
        }
        initContext();
        frameNum = 0;
    }
    if(width != frame.getWidth() || height != frame.getHeight()) {
        fprintf(stderr, "Frames encoded into one video should have the same metrics\n");
        return;
    }
    cv::Mat mat = frame.getImage();
    if(mat.cols > 0 &&  mat.rows > 0)
        writeVideoFrame(mat);
    mat = frame.getAudio();
    if(mat.cols >0 && mat.rows > 0)
        writeAudioFrame(mat);
    frameNum++;
}

void VideoEncoder::writeVideoFrame(const cv::Mat& image, bool flush) {
    if(!flush) {
        cv::imwrite("tmp/test.jpg", image);
        memcpy(ctx->srcPic.data, image.data, image.cols * image.rows * 4);
        ctx->srcPic.linesize[0] = image.cols * 4;
        sws_scale(ctx->sws, ctx->srcPic.data, ctx->srcPic.linesize,
                0, image.rows, ctx->destPic.data, ctx->destPic.linesize);
    }

    AVPacket pkt = {0};
    av_init_packet(&pkt);

    ctx->frame->pts = frameNum;
    int gotPacket;
    if(0 < avcodec_encode_video2(ctx->video_st->codec, &pkt, 
                flush ? NULL : ctx->frame, &gotPacket)) {
        fprintf(stderr, "Encode image failed\n");
        return;
    }
    if(gotPacket) {
        writeFrame(ctx->video_st, pkt);
    }
}

void VideoEncoder::writeAudioFrame(const cv::Mat& audio) {

}

void VideoEncoder::writeFrame(AVStream* stream, AVPacket& pkt) {
    pkt.pts = av_rescale_q_rnd(pkt.pts, stream->codec->time_base, stream->time_base,
            (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt.dts = av_rescale_q_rnd(pkt.dts, stream->codec->time_base, stream->time_base,
            (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt.duration = av_rescale_q(pkt.duration, stream->codec->time_base, stream->time_base);
    if(0 < av_interleaved_write_frame(ctx->ctx, &pkt)) {
        printf("frame!\n");
    }
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
            codecCtx->sample_fmt  = AV_SAMPLE_FMT_FLTP;
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
            codecCtx->channel_layout = AV_CH_LAYOUT_MONO;
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

    if(0 < avpicture_alloc(&(ctx->srcPic), AV_PIX_FMT_BGRA, width, height)) {
        fprintf(stderr, "Could not allocate bgra picture buffer\n");
        return;
    }
    if(0 < avpicture_alloc(&(ctx->destPic), AV_PIX_FMT_YUV420P, width, height)) {
        fprintf(stderr, "Could not allocate yuv420p picture buffer\n");
        return;
    }
    *((AVPicture*)frame) = ctx->destPic;

    // init sws context
    ctx->sws = sws_getContext(width, height, AV_PIX_FMT_BGRA,
            width, height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
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

    // init swr context
    ctx->swr = swr_alloc();
    av_opt_set_int(ctx->swr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(ctx->swr, "in_sample_rate", 24000, 0);
    av_opt_set_sample_fmt(ctx->swr, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    av_opt_set_int(ctx->swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(ctx->swr, "out_sample_rate", 24000, 0);
    av_opt_set_sample_fmt(ctx->swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    if(0 < swr_init(ctx->swr)) {
        fprintf(stderr, "Unable to init swr context\n");
        return;
    }

    // open file
    if(0 < (avio_open(&(ctx->ctx->pb), outputPath.c_str(), AVIO_FLAG_WRITE))) {
        fprintf(stderr, "Can't open output file\n");
        return;
    }
    if(0 < avformat_write_header(ctx->ctx, 0)) {
        fprintf(stderr, "Error writing header\n");
        return;
    }
}

void VideoEncoder::finish() {
    if(!ctx)
        return;
    if(0 < av_write_trailer(ctx->ctx)) {
        fprintf(stderr, "Error writing trailer\n");
        return;
    }
    writeVideoFrame(cv::Mat(), true);
    releaseContext();
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
        swr_free(&(ctx->swr));
    }
    if(ctx->ctx) {
        avio_close(ctx->ctx->pb);
        avformat_free_context(ctx->ctx);
    }
    delete ctx;
    ctx = 0;
}

