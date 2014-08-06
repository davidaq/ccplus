#include "gtest/gtest.h" 
#include "utils.hpp"

TEST(Utils, getImageRotation) {
    EXPECT_EQ(getImageRotation("test/res/test1.jpg"), 0);
    EXPECT_EQ(getImageRotation("test/res/test2.jpg"), 0);
    EXPECT_EQ(getImageRotation("test/res/test3.jpg"), 270);
}

TEST(Utils, dirNameTest) {
    EXPECT_EQ(dirName("test/res/test.tml"), "test/res/");
    EXPECT_EQ(dirName("/res/test.tml"), "/res/");
    EXPECT_EQ(dirName("test.tml"), "");
}
