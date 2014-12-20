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

TEST(Integration, Stop) {
    CCPlus::RenderTarget target;
    target.inputPath = "test/res/test2.tml";
    CCPlus::go(target);
    target.stop();
    target.waitFinish();
}

TEST(Integration, Repeat) {
    CCPlus::RenderTarget target;
    target.inputPath = "test/res/test2.tml";
    CCPlus::go(target);
    CCPlus::go(target);
    CCPlus::go(target);
    CCPlus::RenderTarget target2;
    target2.inputPath = "test/res/basic.tml";
    CCPlus::go(target2);
    target2.waitFinish();
}

TEST(Integration, Player) {
    CCPlus::setRenderMode(PREVIEW_MODE);
    CCPlus::RenderTarget target;
    target.inputPath = "test/res/test2.tml";
    target.mode = PREVIEW_MODE;
    CCPlus::go(target);
    CCPlus::CCPlay::play("tmp", true);
}

TEST(Integration, Player_Stop) {
    CCPlus::setRenderMode(PREVIEW_MODE);
    CCPlus::RenderTarget target;
    target.inputPath = "test/res/test2.tml";
    CCPlus::go(target);
    CCPlus::CCPlay::play("tmp");

    sleep(3);
    CCPlus::CCPlay::stop();
    target.waitFinish();
}
