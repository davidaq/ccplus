#include "gtest/gtest.h"
#include "global.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

TEST(FillTML, GenTML) {
    generateTML("test/res/testfilltml.json", "test/res/render.tml");
    CCPlus::go("test/res/render.tml", "tmp", 18);
}
