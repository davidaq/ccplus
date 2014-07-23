#include "gtest/gtest.h"

#include "ccplus.hpp"

TEST(Integration, SimpleTest) {
    CCPlus::go("test/res/test2.tml", "tmp/integration_simpletest");   
}
