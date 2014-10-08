#include "gtest/gtest.h"

#include "global.hpp"

using namespace CCPlus;

TEST(LayerInterpolate, BasicTest) {
    Layer l("", 0, 0, 0, 0, 800, 600);
    PropertyMap mp;
    Property posProp;
    posProp[1.5].push_back(10.0);
    posProp[4.5].push_back(30.0);
    
    mp["pos"] = posProp;
    l.setProperties(mp);

    std::vector<float> ret = l.interpolate("pos", 3.0);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0], 20);

    ret = l.interpolate("pos", 1.4);
    EXPECT_EQ(ret.size(), 0);

    ret = l.interpolate("pos", 3.75);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.at(0), 25);

    ret = l.interpolate("pos", 2.25);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.at(0), 15);
}
