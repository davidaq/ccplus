#include "gtest/gtest.h"

#include "utils.hpp"

TEST(Utils, getImageRotation) {
    EXPECT_EQ(getImageRotation("test/res/test.jpg"), 0);
}
