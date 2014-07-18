#include "video-decoder.hpp"
#include "image.hpp"

extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
using namespace CCPlus;

// TODO consider mp4 orientation

struct DecodeContext {
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    const char *src_filename = NULL;
    const char *video_dst_filename = NULL;
    const char *audio_dst_filename = NULL;
    FILE *video_dst_file = NULL;
    FILE *audio_dst_file = NULL;
    
    uint8_t *video_dst_data[4] = {NULL};
    int      video_dst_linesize[4];
    int video_dst_bufsize;
    
    int video_stream_idx = -1, audio_stream_idx = -1;
    AVFrame *frame = NULL;
    AVPacket pkt, currentPkt;
    int haveCurrentPkt = 0;
    int video_frame_count = 0;
    int audio_frame_count = 0;
    VideoInfo info;
};

#define CTX (*((DecodeContext*)this->decodeContext))

VideoDecoder::VideoDecoder(const std::string& _inputFile, int _decoderFlag) :
    inputFile(_inputFile),
    decoderFlag(_decoderFlag),
    cursorTime(0),
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

#define TEST_PASS printf("++++ Passed line %d ++++\n", __LINE__);

float VideoDecoder::decodeImage() {
    haveDecodedImage = false;
    int gotFrame = 0;
    float retTime = -1;
    while(!gotFrame) {
        if(!readNextFrameIfNeeded())
            return -1;
        if (CTX.pkt.stream_index == CTX.video_stream_idx) {
            int ret = avcodec_decode_video2(CTX.video_dec_ctx, CTX.frame, &gotFrame, &(CTX.pkt));
            if(ret < 0)
                return -1;
            if(gotFrame) {
                haveDecodedImage = true;
                retTime = av_rescale_q(av_frame_get_best_effort_timestamp(CTX.frame), 
                                        CTX.fmt_ctx->streams[CTX.pkt.stream_index]->time_base, 
                                        AV_TIME_BASE_Q)
                            * 0.000001;
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
    return retTime;
}

Image VideoDecoder::getDecodedImage() {
    if(!haveDecodedImage)
        return Image();
    else {
        av_image_copy(CTX.video_dst_data, CTX.video_dst_linesize,
                      (const uint8_t **)(CTX.frame->data), CTX.frame->linesize,
                      CTX.video_dec_ctx->pix_fmt, CTX.video_dec_ctx->width, CTX.video_dec_ctx->height);
        AVPicture avImg;
        SwsContext* swsContext = sws_getContext(CTX.info.width, CTX.info.height, 
                                                CTX.video_dec_ctx->pix_fmt,
                                                CTX.info.width, CTX.info.height,
                                                PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
        avImg.linesize[0] = CTX.info.width * 4;
        avImg.data[0] = (uint8_t*)malloc(avImg.linesize[0] * CTX.info.height);
        sws_scale(swsContext, CTX.frame->data, CTX.frame->linesize, 0, CTX.info.height, avImg.data, avImg.linesize);
        cv::Mat imgData = cv::Mat(CTX.info.height, CTX.info.width, CV_8UC4, avImg.data[0]);
        return Image(imgData);
    }
}

void VideoDecoder::decodeAudio(float from, float to, const std::string& outputFile) {
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

        int ret = av_image_alloc(CTX.video_dst_data, CTX.video_dst_linesize,
                             CTX.video_dec_ctx->width, CTX.video_dec_ctx->height,
                             CTX.video_dec_ctx->pix_fmt, 1);
        if (ret < 0) {
            releaseContext();
            return;
        }
        CTX.video_dst_bufsize = ret;
        CTX.info.width = CTX.video_dec_ctx->width;
        CTX.info.height = CTX.video_dec_ctx->height;
    }
    if (decoderFlag & DECODE_AUDIO && open_codec_context(&(CTX.audio_stream_idx), CTX.fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        CTX.audio_stream = CTX.fmt_ctx->streams[CTX.audio_stream_idx];
        CTX.audio_dec_ctx = CTX.audio_stream->codec;
    }
    if (!CTX.audio_stream && !CTX.video_stream) {
        releaseContext();
        return;
    }
    
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
    if(CTX.video_dst_data[0])
        av_free(CTX.video_dst_data[0]);
    delete ((DecodeContext*)decodeContext);
    decodeContext = 0;
}