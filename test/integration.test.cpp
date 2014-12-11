#include "gtest/gtest.h"
#include "ccplus.hpp"
#include "global.hpp"

using namespace CCPlus;

TEST(Integration, Basic) {
    CCPlus::go("test/res/basic.tml");
}

TEST(Integration, Simple) {
    CCPlus::go("test/res/test2.tml");
}

extern "C" {
    void CCPLUS_TEST(const char* _opos);
}
TEST(Integration, Shaders) {
    CCPLUS_TEST("tmp/");
}

TEST(Integration, Stop) {
    CCPlus::initContext("test/res/test2.tml");
    CCPlus::render();
    CCPlus::render();

    sleep(2);

    CCPlus::releaseContext();

    CCPlus::render();

    CCPlus::initContext("test/res/test2.tml");
    CCPlus::render();
}
