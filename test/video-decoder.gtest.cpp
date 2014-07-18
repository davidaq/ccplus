#include "gtest/gtest.h" 

#include "video-decoder.hpp"
#include "global.hpp"

using namespace CCPlus;

#define EXPECT_EQF(A,B) EXPECT_TRUE(isnear((A), (B)))

static inline bool isnear(float a, float b) {
    float d = a - b;
    return d < 0.00001 && d > -0.00001;
}

TEST(VideoDecoder, GetVideoInfo) {
    VideoDecoder decoder("test/res/test.mp4");
    VideoInfo info = decoder.getVideoInfo();
    
    EXPECT_EQ(info.width, 640);
    EXPECT_EQ(info.height, 360);
    EXPECT_EQF(info.duration, 10);
}

TEST(VideoDecoder, GoThrough) {
    VideoDecoder decoder("test/res/test.mp4");
    int frames = 0;
    while(decoder.decodeImage() >= 0) {
        frames++;
    }
    frames = (frames + 15) / 25;
    
    EXPECT_EQ(frames, 10);
}

TEST(VideoDecoder, DecodeFrameTime) {
    VideoDecoder decoder("test/res/test.mp4");
    float time;
    for(int i = 0; i < 100; i++) {
        time = decoder.decodeImage();
        
        EXPECT_EQF(time, 0.04 * i);
    }
}

TEST(VideoDecoder, DecodeFrameImage) {
    VideoDecoder decoder("test/res/test.mp4");
    decoder.decodeImage();
    Image img = decoder.getDecodedImage();
    
    EXPECT_EQ(img.getWidth(), 640);
    EXPECT_EQ(img.getHeight(), 360);
    EXPECT_EQ(img.getData().data[0], 0xb5);
    EXPECT_EQ(img.getData().data[1], 0xbc);
    EXPECT_EQ(img.getData().data[2], 0xc2);
}