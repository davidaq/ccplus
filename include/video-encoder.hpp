#pragma once
#include <string>

namespace CCPlus {
    class VideoEncoder;
    class Image;
    struct DecodeContext;
}

class CCPlus::VideoEncoder {
public:
    VideoEncoder(const std::string& outputPath, int fps);
    ~VideoEncoder();

    // @ must be set before appending images
    void setAudio(const char* rawAudioPath);

    // @ append a frame
    void appendImage(const CCPlus::Image& frame);

    // @ finish the encoding, the object should not be used after this
    // will be called on destruct
    void finish();
private:
    void initContext();
    void releaseContext();
#ifdef __FFMPEG__
    AVStream* initStream(AVCodec*&, enum AVCodecID);
#endif

    DecodeContext *ctx = 0;
    int width = 0, height = 0, fps = 0;
    std::string outputPath;
};
