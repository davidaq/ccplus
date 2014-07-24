#include "video-encoder.hpp"
#include "global.hpp"

using namespace CCPlus;

VideoEncoder::VideoEncoder(const char* outputPath, int fps) {

}

VideoEncoder::~VideoEncoder() {
    finish();
}

void VideoEncoder::appendImage(const Image& frame) {

}

void VideoEncoder::finish() {
    if(!decodeContext)
        return;
}

void VideoEncoder::initContext() {
    if(decodeContext)
        return;
}

void VideoEncoder::releaseContext() {
    if(!decodeContext)
        return;
}

