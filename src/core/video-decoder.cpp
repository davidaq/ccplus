#include "video-decoder.hpp"

using namespace CCPlus;

VideoDecoder::VideoDecoder(const std::string& _inputFile) :
    inputFile(_inputFile) {
}

void VideoDecoder::seekTo(float time) {
}

float VideoDecoder::decodeImage() {

}

VideoInfo VideoDecoder::getVideoInfo() {
    return VideoInfo {
        .duration = 0,
        .width = 0,
        .height = 0
    };
}

void VideoDecoder::decodeAudio(float from, float to, const std::string& outputFile) {
}