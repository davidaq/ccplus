#include "gtest/gtest.h"

#include "frame.hpp"
#include "blend-utils.hpp"
#include "utils.hpp"
#include "logger.hpp"

using namespace CCPlus;

TEST(BlendTest, ALL) {
    for (int i = -1; i < 9; i++)
    {
        Frame fg = Frame("test/res/blend_fg.jpg");
        Frame bg = Frame("test/res/blend_bg.jpg");
        fg.mergeFrame(bg, i);
        fg.write("tmp/blend_" + toString(i) + ".jpg");
    }
}
