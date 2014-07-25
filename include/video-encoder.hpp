#pragma once
#include <string>
#include <opencv2/opencv.hpp>

#define Frame TFRAME

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
        cv::Mat getImage() const {
            cv::Mat ret = cv::Mat(6, 6, CV_8UC4, cv::Scalar(0, 0, 0, 0));
            ret.at<cv::Vec4b>(3, 3)[0] = 255;
            ret.at<cv::Vec4b>(3, 3)[1] = 255;
            ret.at<cv::Vec4b>(3, 3)[2] = 255;
            ret.at<cv::Vec4b>(3, 3)[3] = 255;
            return ret;
        }
        cv::Mat getAudio() const {
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
    void writeVideoFrame(const cv::Mat&, bool flush=false);
    void writeAudioFrame(const cv::Mat&);
    void writeFrame(AVStream* stream, AVPacket& pkt);
#endif

    EncodeContext *ctx = 0;
    std::string outputPath;
    int width = 0, height = 0, fps = 0;
    int frameNum = 0;
};
