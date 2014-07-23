#pragma once

#include "image.hpp"

namespace CCPlus {
    class VideoDecoder;
    struct VideoInfo;
}

struct CCPlus::VideoInfo {
    float duration;
    int width, height;
};

class CCPlus::VideoDecoder {
public:
    VideoDecoder(const std::string& inputFile, int decoderFlag=CCPlus::VideoDecoder::DECODE_VIDEO|CCPlus::VideoDecoder::DECODE_AUDIO);
    ~VideoDecoder();
    
    const static int DECODE_VIDEO = 1, DECODE_AUDIO = 2;
    
    // Retrieve video basic information
    VideoInfo getVideoInfo();
    
    // Sets the cursor to a specific time (in seconds)
    void seekTo(float time);
    
    // Try to decode an image frame at the current cursor poisition
    // @return the time (in seconds) of the decoded frame
    // @return negative value if no frame can be decoded
    float decodeImage();
    
    // @return the frame image in the previous successful decode from calling decodeImage()
    CCPlus::Image getDecodedImage();
    
    // Decode audio stream into PCM raw data from current position
    // Can be used for both video & audio files
    // Output writen to file
    void decodeAudio(const std::string& outputFile, float durationLimit = -1);
    
private:
    std::string inputFile;
    int decoderFlag;
    float cursorTime = 0;
    void* decodeContext = 0;
    bool haveDecodedImage = false;
    
    void initContext();
    void releaseContext();
    bool readNextFrameIfNeeded();

    void decodeAudio(FILE* destFile, float durationLimit);
    int decodeAudioFrame(FILE* destFile, float durationLimit, float &start, float &gap);
};
