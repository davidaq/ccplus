#include "gtest/gtest.h"
#include "ccplus.hpp"
#include "global.hpp"

using namespace CCPlus;

TEST(Integration, Basic) {
    CCPlus::go("test/res/basic.tml", "tmp", 18);
}

TEST(Integration, Stop) {
    CCPlus::initContext("test/res/test2.tml", "tmp", 18);
    CCPlus::render();
    CCPlus::render();

    sleep(2);

    CCPlus::releaseContext();

    CCPlus::render();
    CCPlus::encode();

    CCPlus::initContext("test/res/test2.tml", "tmp", 18);
    CCPlus::render();
    CCPlus::encode();
}
