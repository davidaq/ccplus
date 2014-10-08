#include "gtest/gtest.h"

#include "global.hpp"

using namespace CCPlus;

TEST(RenderableTest, getFrameNumberTest) {
    Context* ctx = Context::getInstance();
    ctx->init(".", 4);
    Composition r("", 0, 0, 0);
    
    EXPECT_EQ(r.getFrameNumber(0.124), 0);
    EXPECT_EQ(r.getFrameNumber(0.125), 1);
    EXPECT_EQ(r.getFrameNumber(0.5), 2);
    EXPECT_EQ(r.getFrameNumber(0.77), 3);
    EXPECT_EQ(r.getFrameNumber(2), 8);
}
