#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

TEST(Context, Constructor) {
    Context ctx("res");
    EXPECT_EQ(ctx.getStoragePath(), "res");
}