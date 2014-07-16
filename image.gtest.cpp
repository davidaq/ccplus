#include <gtest/gtest.h>
#include "global.hpp"
#include "ios"

using namespace CCPlus;

TEST(Image, ImageRead) {
    Image img("test/res/test.png");
    EXPECT_EQ(img.getHeight(), 1080);
    EXPECT_EQ(img.getWidth(), 1920);

    EXPECT_THROW(Image("hithere"), std::ios_base::failure);
}
