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
    EXPECT_FILE_HASH_EQ(edd5d2c3743cf5bc5dc46d25c329988ba5467373, tmp/decode-encodetest.mp4);
}

TEST(VideoEncoder, DecodeImageAndAudioThenEncode) {
    VideoEncoder encoder("tmp/decode-encode-with-audio.mp4", 18);

    Context context("tmp/", 18);
    VideoRenderable renderable(&context, "test/res/test.mp4");
    renderable.render(0, 10);
    float t = 0;
    float step = 1.0 / 18;
    do {
        encoder.appendFrame(renderable.getFrame(t));
        t += step;
    } while(t < 10);
    encoder.finish();
}

TEST(VideoEncoder, EncodeAudioOnly) {
    VideoEncoder encoder("tmp/encode-only-audio.mp4", 18);
    FILE* fp = fopen("test/res/test.pcm", "rb");
    int frameSize = 1333;
    short *buff = new short[frameSize];
    cv::Mat img = cv::Mat(40, 40, CV_8UC4, {0,0,0,0});
    Frame frame(img);
    while(0 < fread(buff, frameSize * 2, 1, fp)) {
        std::vector<short> vec(buff, buff + frameSize);
        cv::Mat audio = cv::Mat(vec);
        frame.setAudio(audio);
        encoder.appendFrame(frame);
    }
    encoder.finish();
    delete []buff;
}

