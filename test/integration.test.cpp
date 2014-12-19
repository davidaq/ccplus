#include "gtest/gtest.h"
#include "ccplus.hpp"
#include "ccplay.hpp"
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
    CCPlus::RenderTarget target;
    target.tmlPath = "test/res/test2.tml";
    CCPlus::go(&target);
    target.stop();
    target.waitFinish();
}

TEST(Integration, Repeat) {
    CCPlus::RenderTarget target;
    target.tmlPath = "test/res/test2.tml";
    CCPlus::go(&target);
    CCPlus::go(&target);
    CCPlus::go(&target);
    CCPlus::RenderTarget target2;
    target2.tmlPath = "test/res/basic.tml";
    CCPlus::go(&target2);
    target2.waitFinish();
}

TEST(Integration, Player) {
    CCPlus::setRenderMode(PREVIEW_MODE);
    CCPlus::RenderTarget target;
    target.tmlPath = "test/res/test2.tml";
    CCPlus::go(&target);
    CCPlus::CCPlay::play("tmp", 18, true);
}

TEST(Integration, Player_Stop) {
    CCPlus::setRenderMode(PREVIEW_MODE);
    CCPlus::RenderTarget target;
    target.tmlPath = "test/res/test2.tml";
    CCPlus::go(&target);
    CCPlus::CCPlay::play("tmp", 18);

    sleep(3);
    CCPlus::CCPlay::stop();
    target.waitFinish();
}

//TEST(Integration, Player_Rewind) {
//    CCPlus::setRenderMode(PREVIEW_MODE);
//    CCPlus::RenderTarget target;
//    target.tmlPath = "test/res/test2.tml";
//    CCPlus::go(&target);
//    CCPlus::play("tmp", 18);
//
//    sleep(6);
//    CCPlus::rewind();
//    sleep(15);
//}
