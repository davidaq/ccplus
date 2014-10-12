#include "gtest/gtest.h"
#include "ccplus.hpp"

TEST(Integration, Basic) {
    CCPlus::go("test/res/basic.tml", "tmp", 18);
}
