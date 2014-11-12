#include "gtest/gtest.h"
#include "global.hpp"
#include "video-decoder.hpp"

using namespace CCPlus;

TEST(Video, Decoder) {
    VideoDecoder decoder("test/res/glitch.mov");
    decoder.seekTo(0);
    float pos;
    while((pos = decoder.decodeImage()) + 0.001 > 0) {
        L() << pos;
        char fname[20];
        sprintf(fname, "tmp/glitch%f.jpg", pos);
        cv::imwrite(fname, decoder.getDecodedImage().image);
    }
}

TEST(Video, Decoder2) {
    VideoDecoder decoder("test/res/screen.mov");
    decoder.seekTo(0);
    float pos;
    while((pos = decoder.decodeImage()) + 0.001 > 0) {
        L() << pos;
        char fname[20];
        sprintf(fname, "tmp/screen%f.jpg", pos);
        cv::imwrite(fname, decoder.getDecodedImage().image);
    }
}
