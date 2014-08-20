#define __STDC_FORMAT_MACROS
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

#include "global.hpp"
#include "video-encoder.hpp"

using namespace CCPlus;

struct CCPlus::EncodeContext {
    AVOutputFormat *fmt = 0;
    AVFormatContext *ctx = 0;
    AVStream *audio_st, *video_st = 0;
    AVCodec *audio_codec, *video_codec = 0;

    AVFrame *frame = 0;
    AVPicture destPic;
    SwsContext *sws = 0;

    AVFrame *audioFrame = 0;
    SwrContext *swr = 0;
    int samples_count = 0;
    uint8_t **audioDstBuff = 0;
    int audioDstLinesize = 0, audioDstSamplesSize = 0;
    size_t currentAudioBuffSize = 0;
    int audioFrameSize = 0;
    uint8_t *audioPendingBuff = 0;
    int audioPendingBuffLen = 0;
    int bytesPerSample = 2;

};

VideoEncoder::VideoEncoder(const std::string& _outputPath, int _fps, int _quality) :
    outputPath(_outputPath), fps(_fps), quality(_quality)
{
}

VideoEncoder::~VideoEncoder() {
    finish();
}

void VideoEncoder::appendFrame(const Frame& frame) {
    if(!ctx) {
        width = frame.getWidth();
        height = frame.getHeight();
        if(width == 0 || height == 0)
            return;
        if(width & 1)
            width += 1;
        if(height & 1)
            height += 1;
        initContext();
        frameNum = 0;
    }
    cv::Mat img;
    if(frame.getWidth() == 0 || frame.getHeight() == 0) {
        img = cv::Mat::zeros(height, width, CV_8UC4);
    } else
        img = frame.getImage();
    writeVideoFrame(img);
    cv::Mat mat = frame.getAudio();
    if(mat.total() > 0) {
        writeAudioFrame(mat);
    }
    frameNum++;
}

void VideoEncoder::writeVideoFrame(const cv::Mat& image, bool flush) {
    if(!flush) {
        cv::Mat frame = image;
        if(image.cols != width || image.rows != height) {
            cv::resize(image, frame, cv::Size(width, height));
        }
        int linesize = frame.cols * 4;
        sws_scale(ctx->sws, &frame.data, &linesize,
                0, frame.rows, ctx->destPic.data, ctx->destPic.linesize);
    }

    AVPacket pkt = {0};
    av_init_packet(&pkt);

    ctx->frame->pts = frameNum;
    int gotPacket;
    if(0 > avcodec_encode_video2(ctx->video_st->codec, &pkt, 
                flush ? NULL : ctx->frame, &gotPacket)) {
        fprintf(stderr, "Encode image failed\n");
        return;
    }
    if(gotPacket) {
        writeFrame(ctx->video_st, pkt);
    }
}

void VideoEncoder::writeAudioFrame(const cv::Mat& audio, bool flush) {
    if(flush) {
        AVPacket pkt = {0};
        av_init_packet(&pkt);
        int gotPacket;
        if(0 > avcodec_encode_audio2(ctx->audio_st->codec, &pkt, NULL, &gotPacket)) {
            fprintf(stderr, "Error encoding audio\n");
            return;
        }
        if(gotPacket) {
            writeFrame(ctx->audio_st, pkt);
        }
    } else {
        int income = audio.total() * ctx->bytesPerSample;
        int audioFrameByteSize = ctx->audioFrameSize * ctx->bytesPerSample;
        if(ctx->audioPendingBuffLen + income < audioFrameByteSize) {
            memcpy(ctx->audioPendingBuff + ctx->audioPendingBuffLen, audio.data, income);
            ctx->audioPendingBuffLen += income;
        } else {
            uint8_t* ptr = audio.data;
            if(ctx->audioPendingBuffLen > 0) {
                int d = audioFrameByteSize - ctx->audioPendingBuffLen;
                memcpy(ctx->audioPendingBuff + ctx->audioPendingBuffLen, audio.data, d);
                writePartedAudioFrame(ctx->audioPendingBuff);
                ptr += d;
                income -= d;
            }
            while(income >= audioFrameByteSize) {
                writePartedAudioFrame(ptr);
                ptr += audioFrameByteSize;
                income -= audioFrameByteSize;
            }
            if(income > 0) {
                memcpy(ctx->audioPendingBuff, ptr, income);
            }
            ctx->audioPendingBuffLen = income;
        }
    }
}

void VideoEncoder::writePartedAudioFrame(const uint8_t* sampleBuffer) {
    AVPacket pkt = {0};
    av_init_packet(&pkt);

    int nb_samples = ctx->audioFrameSize;
    AVCodecContext* c = ctx->audio_st->codec;

    if(ctx->currentAudioBuffSize < nb_samples) {
        ctx->currentAudioBuffSize = nb_samples;
        if(ctx->audioDstBuff) {
            av_free(ctx->audioDstBuff[0]);
        }
        if(0 > av_samples_alloc_array_and_samples(&(ctx->audioDstBuff), &(ctx->audioDstLinesize), 1,
                    nb_samples, AV_SAMPLE_FMT_FLTP, 0)) {
            fprintf(stderr, "Error alloc audio dest buffer\n");
            return;
        }
        ctx->audioDstSamplesSize = av_samples_get_buffer_size(NULL, c->channels, nb_samples, c->sample_fmt, 0);
    }
    size_t linesize = nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_FLTP);
    swr_convert(ctx->swr, ctx->audioDstBuff, linesize, &sampleBuffer, nb_samples);

    ctx->audioFrame->nb_samples = nb_samples;
    ctx->audioFrame->pts = av_rescale_q(ctx->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
    ctx->samples_count += nb_samples;
    avcodec_fill_audio_frame(ctx->audioFrame, c->channels, c->sample_fmt, ctx->audioDstBuff[0], ctx->audioDstSamplesSize, 0);

    int gotPacket;
    if(0 > avcodec_encode_audio2(c, &pkt, ctx->audioFrame, &gotPacket)) {
        fprintf(stderr, "Error encoding audio\n");
        return;
    }
    if(gotPacket) {
        writeFrame(ctx->audio_st, pkt);
    }
}

void VideoEncoder::writeFrame(AVStream* stream, AVPacket& pkt) {
    pkt.stream_index = stream->id;
    pkt.pts = av_rescale_q_rnd(pkt.pts, stream->codec->time_base, stream->time_base,
            (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt.dts = av_rescale_q_rnd(pkt.dts, stream->codec->time_base, stream->time_base,
            (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt.duration = av_rescale_q(pkt.duration, stream->codec->time_base, stream->time_base);
    if(stream->pts.den == 0)
        stream->pts.den = 1;
    if(0 > av_interleaved_write_frame(ctx->ctx, &pkt)) {
        printf("Bad frame!\n");
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
            if(quality >= 300)
                codecCtx->bit_rate    = 512000;
            else if(quality >= 200)
                codecCtx->bit_rate    = 256000;
            else if(quality >= 100)
                codecCtx->bit_rate    = 128000;
            else if(quality >= 70)
                codecCtx->bit_rate    = 64000;
            else if(quality >= 40)
                codecCtx->bit_rate    = 32000;
            else
                codecCtx->bit_rate    = 16000;

            codecCtx->sample_rate = CCPlus::AUDIO_SAMPLE_RATE;
            codecCtx->channels        = 1;
            codecCtx->channel_layout  = AV_CH_LAYOUT_MONO;
            break;
        case AVMEDIA_TYPE_VIDEO:
            codecCtx->codec_id = codec_id;
            codecCtx->bit_rate = std::sqrt(width * height) * 300 * quality;
            codecCtx->width    = width;
            codecCtx->height   = height;

            codecCtx->gop_size      = 12; 
            codecCtx->pix_fmt       = AV_PIX_FMT_YUV420P;
            codecCtx->time_base.den = fps;
            codecCtx->time_base.num = 1;
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
    if(0 > avcodec_open2(c, ctx->video_codec, NULL)) {
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

    if(0 > avpicture_alloc(&(ctx->destPic), AV_PIX_FMT_YUV420P, width, height)) {
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
    if(0 > avcodec_open2(c, ctx->audio_codec, &opts)) {
        fprintf(stderr, "Could not open audio encoder\n");
        return;
    }
    av_dict_free(&opts);
    ctx->audioFrame = av_frame_alloc();
    if(!ctx->audioFrame) {
        fprintf(stderr, "Could not alloc audio frame\n");
        return;
    }
    ctx->audioFrameSize = ctx->audio_codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE ?
        2048 : ctx->audio_st->codec->frame_size;
    ctx->audioPendingBuff = new uint8_t[ctx->audioFrameSize * ctx->bytesPerSample + 10];

    // init swr context
    ctx->swr = swr_alloc();
    av_opt_set_int(ctx->swr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(ctx->swr, "in_sample_rate", CCPlus::AUDIO_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(ctx->swr, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    av_opt_set_int(ctx->swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(ctx->swr, "out_sample_rate", CCPlus::AUDIO_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(ctx->swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    if(0 > swr_init(ctx->swr)) {
        fprintf(stderr, "Unable to init swr context\n");
        return;
    }

    // open file
    if(0 > (avio_open(&(ctx->ctx->pb), outputPath.c_str(), AVIO_FLAG_WRITE))) {
        fprintf(stderr, "Can't open output file\n");
        return;
    }
    if(0 > avformat_write_header(ctx->ctx, 0)) {
        fprintf(stderr, "Error writing header\n");
        return;
    }
}

void VideoEncoder::finish() {
    if(!ctx)
        return;
    writeVideoFrame(cv::Mat(), true);
    writeAudioFrame(cv::Mat(), true);
    if(0 > av_write_trailer(ctx->ctx)) {
        fprintf(stderr, "Error writing trailer\n");
        return;
    }
    releaseContext();
}

void VideoEncoder::releaseContext() {
    if(!ctx)
        return;
    if(ctx->video_st) {
        avcodec_close(ctx->video_st->codec);
        av_free(ctx->destPic.data[0]);
        av_frame_free(&(ctx->frame));
        sws_freeContext(ctx->sws);
    }
    if(ctx->audio_st) {
        avcodec_close(ctx->audio_st->codec);
        av_frame_free(&(ctx->audioFrame));
        swr_free(&(ctx->swr));
        if(ctx->audioDstBuff) {
            av_free(ctx->audioDstBuff[0]);
            av_free(ctx->audioDstBuff);
        }
        av_frame_free(&(ctx->audioFrame));
    }
    if(ctx->ctx) {
        avio_close(ctx->ctx->pb);
        avformat_free_context(ctx->ctx);
    }
    if(ctx->audioPendingBuff)
        delete [] ctx->audioPendingBuff;
    delete ctx;
    ctx = 0;
}

