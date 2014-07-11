#include "gtest/gtest.h" 

#include "tmlreader.hpp"
#include "composition.hpp"

using namespace CCPlus;

TEST(demotest, boringtest)  {
    EXPECT_EQ(1, 1);
}

TEST(TMLReaderTest, BasicTest) {
    Composition mainComp = TMLReader::read("res/test1.tml");
    EXPECT_EQ(mainComp.getName(), "MAIN");
    /*
    EXPECT_EQ(comp.getVersion(), "0.01");
    EXPECT_EQ(comp.getResolution(), {1920, 1080});
    EXPECT_EQ(comp.getDuration(), 10.0);
    */
}
