#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

TEST(Context, Constructor) {
    Context* ctx = Context::getInstance();
    ctx->init("res", 18);
    EXPECT_EQ(ctx->getStoragePath(), "res");
    EXPECT_EQ(ctx->getFPS(), 18);
}
