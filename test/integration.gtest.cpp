#include "gtest/gtest.h"

#include "ccplus.hpp"

TEST(Integration, SimpleTest) {
    CCPlus::go("test/res/test2.tml", "tmp/", 0.0, 10.0);   
    //CCPlus::go("test/res/test4.tml", "tmp/", 0.0, 60.0, 25);   
}

