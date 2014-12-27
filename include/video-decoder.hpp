#pragma once

#include "global.hpp"
#include <vector>
#include "frame.hpp"

namespace CCPlus {
    struct VideoInfo;
    struct DecodeContext;
}

struct CCPlus::VideoInfo {
    float duration;
    int width, height;
    int rwidth, rheight;
};


#ifdef VIDEO_DECODER
#define OuputF std::function<const void*, size_t, size_t>
#endif

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
    CCPlus::Frame getDecodedImage();
    
    // Decode audio stream into PCM raw data from current position
    // Can be used for both video & audio files
    // Output writen to file
    void decodeAudio(const std::string& outputFile, float durationLimit = -1);
    // Output to a vec
    float decodeAudio(std::vector<int16_t>& vec, float durationLimit = -1);
    std::vector<int16_t> decodeAudio(float durationLimit = -1);

    bool invalid = true;
    
private:
    std::string inputFile;
    int decoderFlag;
    float cursorTime = 0;
    CCPlus::DecodeContext* decodeContext = 0;
    bool haveDecodedImage = false;
    Frame* decodedImage = 0;
    
    void initContext();
    void releaseContext();
    bool readNextFrameIfNeeded();

    float decodeAudio(std::function<void(const void*, size_t, size_t)> output, float durationLimit);
    void decodeAudio(FILE* destFile, float durationLimit);
    int decodeAudioFrame(std::function<void(const void*, size_t, size_t)> output, float durationLimit, float &start, float &gap, float* realTime=0);
};

