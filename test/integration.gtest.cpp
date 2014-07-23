#include "gtest/gtest.h"

#include "ccplus.hpp"

TEST(Integration, SimpleTest) {
    CCPlus::go("test/res/test2.tml", "tmp/", 0.0, 1.0);   
}
