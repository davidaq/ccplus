#pragma once
#include "global.hpp"
#include <string>
#include <list>
#include <opencv2/opencv.hpp>

namespace CCPlus {
    struct EncodeContext;
}

class CCPlus::VideoEncoder {
public:
    VideoEncoder(const std::string& outputPath, int fps, int width=0, int height=0, int quality=70);
    ~VideoEncoder();

    // @ append a frame
    void appendFrame(const CCPlus::Frame& frame);

    // @ finish the encoding, the object should not be used after this
    // will be called on destruct
    void finish();
    inline int getWidth() const {
        return width;
    }
    inline int getHeight() const {
        return height;
    }

private:
#ifdef NEVER_DEFINED
    const void* encodeContext;
#else
    void initContext();
    void releaseContext();
#ifdef __FFMPEG__
    AVStream* initStream(AVCodec*&, enum AVCodecID);
    void writeVideoFrame(const cv::Mat&, bool flush=false);
    void writeAudioFrame(const cv::Mat&, bool flush=false);
    void writePartedAudioFrame(const uint8_t* sampleBuffer);
    void writeFrame(AVStream* stream, AVPacket& pkt);
#endif

    EncodeContext *ctx = 0;
    std::string outputPath;
    int width = 0, height = 0, fps = 0, quality = 100;
    int frameNum = 0;
    bool finished = false;

    pthread_t workerThread = 0;
    void doAppendFrame(const Frame& frame);
    std::list<CCPlus::Frame> queue;
    CCPlus::Semaphore queueInSync, queueOutSync;
    CCPlus::Lock queueLock;
#endif

};
