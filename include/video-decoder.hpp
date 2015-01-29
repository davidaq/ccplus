#pragma once

#include "global.hpp"
#include <vector>
#include "frame.hpp"

namespace CCPlus {
    struct VideoInfo;
    struct DecodeContext;
    IVideoDecoderRef openDecoder(std::string uri, int flags, bool isUser=false);
};

struct CCPlus::VideoInfo {
    float duration;
    int width, height;
    int rwidth, rheight;
};


class CCPlus::IVideoDecoder : public CCPlus::Object {
public:
    virtual VideoInfo getVideoInfo() = 0;
    
    // Sets the cursor to a specific time (in seconds)
    virtual void seekTo(float time, bool realSeek = true) = 0;
    
    // Try to decode an image frame at the current cursor poisition
    // @return the time (in seconds) of the decoded frame
    // @return negative value if no frame can be decoded
    virtual float decodeImage() = 0;
    
    // @return the frame image in the previous decode from calling decodeImage()
    virtual CCPlus::Frame getDecodedImage(int maxSize=512) = 0;
    
    // Decode audio stream into PCM raw data from current position
    // Can be used for both video & audio files
    virtual float decodeAudio(std::vector<int16_t>& vec, float durationLimit = -1) = 0;
    inline std::vector<int16_t> decodeAudio(float durationLimit = -1) {
        std::vector<int16_t> ret;
        decodeAudio(ret, durationLimit);
        return ret;
    };
};

class CCPlus::VideoDecoder : public CCPlus::IVideoDecoder {
public:
    VideoDecoder(const std::string& inputFile, int decoderFlag=CCPlus::VideoDecoder::DECODE_VIDEO|CCPlus::VideoDecoder::DECODE_AUDIO);
    ~VideoDecoder();
    
    const static int DECODE_VIDEO = 1, DECODE_AUDIO = 2;
    
    VideoInfo getVideoInfo();
    
    void seekTo(float time, bool realSeek = true);
    float decodeImage();
    
    CCPlus::Frame getDecodedImage(int maxSize=512);

    float decodeAudio(std::vector<int16_t>& vec, float durationLimit = -1);

    bool invalid = true;
    
private:
#ifdef VIDEO_DECODER
#define OuputF std::function<const void*, size_t, size_t>
#endif
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
    int decodeAudioFrame(std::function<void(const void*, size_t, size_t)> output, float& time);
};

