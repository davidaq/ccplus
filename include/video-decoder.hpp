#pragma once

#include <string>
#include <opencv2/opencv.hpp>

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
    VideoDecoder(const std::string& inputFile);
    
    // Retrieve video basic information
    VideoInfo getVideoInfo();
    
    // Sets the cursor to a specific time (in seconds)
    void seekTo(float time);
    
    // Try to decode an image frame at the current cursor poisition
    // @return the time (in seconds) of the decoded frame
    // @return -1 if no frame can be decoded
    float decodeImage();

    // Writes the last decoded image frame to an output file
    void writeLastDecodedImageDataTo(const std::string& outputFile);
    
    // Writes the frame before the last decoded image frame to an output file
    void writePreviousDecodedImageDataTo(const std::string& outputFile);
    
    // Decode audio stream into PCM raw data, and write to outputFile
    // Can be used for both video & audio files
    void decodeAudio(float from, float to, const std::string& outputFile);
    
private:
    std::string inputFile;
    float cursorTime;
};