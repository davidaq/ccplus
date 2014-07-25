#include "video-encoder.hpp"
#include <gtest/gtest.h>

using namespace CCPlus;

TEST(VideoEncoder,EncodeImageOnly) {
    VideoEncoder encoder("tmp/encodetest.mp4", 15);
    encoder.appendFrame(Frame());
}

