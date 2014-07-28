#include <gtest/gtest.h>
#include "video-encoder.hpp"
#include "global.hpp"

using namespace CCPlus;

TEST(VideoEncoder,EncodeImageOnly) {
    VideoEncoder encoder("tmp/encodetest.mp4", 15);
    for(int i = 0; i < 50; i++)
        encoder.appendFrame(Frame());
    encoder.finish();
}

