#pragma once
#include <string>
#include <opencv2/opencv.hpp>

namespace CCPlus {
    class VideoEncoder;
    struct EncodeContext;

    class Frame {
    public:
        int getWidth() const {
            return 6;
        }
        int getHeight() const {
            return 6;
        }
        cv::Mat getImage() {
            return cv::Mat();
        }
        cv::Mat getAudio() {
            return cv::Mat(std::vector<int16_t>());
        }
        
    };

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
#endif

    EncodeContext *ctx = 0;
    std::string outputPath;
    int width = 0, height = 0, fps = 0;
};
