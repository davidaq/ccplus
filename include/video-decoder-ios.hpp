#pragma once

#include "video-decoder.hpp"

namespace CCPlus {
    class VideoDecoderIOS;
};

class CCPlus::VideoDecoderIOS : public CCPlus::IVideoDecoder {
public:
    VideoDecoderIOS(const std::string& inputFile);
    ~VideoDecoderIOS();

    VideoInfo getVideoInfo();
    
    void seekTo(float time, bool realSeek = true);
    
    float decodeImage();
    
    CCPlus::Frame getDecodedImage(int maxSize=512);
    
    float decodeAudio(std::vector<int16_t>& vec, float durationLimit = -1);
private:
    const void* decodeContext = 0;
};
