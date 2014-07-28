#pragma once
#include <string>
#include <opencv2/opencv.hpp>


namespace CCPlus {
    class VideoEncoder;
    struct EncodeContext;
    class Frame;
}

class CCPlus::VideoEncoder {
public:
    VideoEncoder(const std::string& outputPath, int fps);
    ~VideoEncoder();

    // @ append a frame
    void appendFrame(const CCPlus::Frame& frame);

    // @ finish the encoding, the object should not be used after this
    // will be called on destruct
    void finish();
private:
    void initContext();
    void releaseContext();
#ifdef __FFMPEG__
    AVStream* initStream(AVCodec*&, enum AVCodecID);
    void writeVideoFrame(const cv::Mat&, bool flush=false);
    void writeAudioFrame(const cv::Mat&);
    void writeFrame(AVStream* stream, AVPacket& pkt);
#endif

    EncodeContext *ctx = 0;
    std::string outputPath;
    int width = 0, height = 0, fps = 0;
    int frameNum = 0;
};
