#include <gtest/gtest.h>
#include "testext.h"

#include "video-encoder.hpp"
#include "video-decoder.hpp"
#include "global.hpp"

using namespace CCPlus;

TEST(VideoEncoder, EncodeStaticImage) {
    VideoEncoder encoder("tmp/encodetest.mp4", 15);
    Frame frame("test/res/test_alpha.png");
    for(int i = 0; i < 50; i++)
        encoder.appendFrame(frame);
    encoder.finish();
    EXPECT_FILE_HASH_EQ(408cf845fecf1e2f81fa0d062572c9b559fd3a42, tmp/encodetest.mp4);
}

TEST(VideoEncoder, DecodeAndEncode) {
    VideoDecoder decoder("test/res/test.mp4");
    VideoEncoder encoder("tmp/decode-encodetest.mp4", 18);
    decoder.seekTo(4);
    while(0 <= decoder.decodeImage()) {
        encoder.appendFrame(decoder.getDecodedImage());
    }
    encoder.finish();
    EXPECT_FILE_HASH_EQ(bd7c8c73c6f28464929401a03dc5e7c739c74962, tmp/decode-encodetest.mp4);
}
