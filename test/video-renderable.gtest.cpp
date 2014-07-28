#include "gtest/gtest.h"

#include "video-renderable.hpp"
#include "global.hpp"

using namespace CCPlus;

TEST(VideoRenderable, NormalTest) {
    Context ctx("tmp/", 18);
    VideoRenderable r(&ctx, "test/res/test.mp4");   
    r.render(0, 10.0);
}

TEST(VideoRenderable, OnlyAudio) {
    Context ctx("tmp/", 18);
    VideoRenderable r(&ctx, "test/res/test.m4a");   
    r.render(0, 10.0);
    EXPECT_EQ(0, r.getWidth());
    EXPECT_EQ(0, r.getHeight());
    EXPECT_NEAR(10, r.getDuration(), 1);
    EXPECT_EQ(1333, r.getFrame(0).getAudio().total());
    EXPECT_TRUE(r.getFrame(10).getImage().empty());
}
