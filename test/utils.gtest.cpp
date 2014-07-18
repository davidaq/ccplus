#include "gtest/gtest.h" 
#include "utils.hpp"

TEST(Utils, getImageRotation) {
    EXPECT_EQ(getImageRotation("test/res/test1.jpg"), 0);
    EXPECT_EQ(getImageRotation("test/res/test2.jpg"), 0);
}
