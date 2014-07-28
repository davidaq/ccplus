#include <gtest/gtest.h>
#include "testext.h"

#include "video-renderable.hpp"
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

TEST(VideoEncoder, DecodeImageAndEncode) {
    VideoDecoder decoder("test/res/test.mp4");
    VideoEncoder encoder("tmp/decode-encodetest.mp4", 18);
    decoder.seekTo(4);
    while(0 <= decoder.decodeImage()) {
        encoder.appendFrame(decoder.getDecodedImage());
    }
    encoder.finish();
    EXPECT_FILE_HASH_EQ(d5b035b09af07a4865534228cd56e16900cedebf, tmp/decode-encodetest.mp4);
}

TEST(VideoEncoder, DecodeImageAndAudioThenEncode) {
    VideoEncoder encoder("tmp/decode-encode-with-audio.mp4", 18);

    Context context("tmp/", 18);
    VideoRenderable renderable(&context, "test/res/test.mp4");
    renderable.render(0, 10);
    float t = 0;
    float step = 1.0 / 18;
    while(t < 10) {
        t += step;
        encoder.appendFrame(renderable.getFrame(t));
    }
    encoder.finish();
}

