#pragma once

namespace CCPlus {
    class VideoEncoder;
    class Image;
    class DecodeContext;
}

class CCPlus::VideoEncoder {
public:
    VideoEncoder(const char* outputPath, int fps);
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
    DecodeContext *decodeContext = 0;
};
