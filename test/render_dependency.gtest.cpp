#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

TEST(CompositionDependencyTest, DirectDependency) {
    Context ctx("");
    TMLReader reader(&ctx);
    Composition* mainComp = reader.read("test/res/test1.tml");
    EXPECT_EQ(mainComp->directDependency(0, 5).size(), 1);
    EXPECT_EQ(mainComp->directDependency(0, 0).size(), 0);
    Composition* comp = dynamic_cast<Composition*>(mainComp->directDependency(0, 5)[0].renderable);
    EXPECT_TRUE(comp != 0);
    if(comp != 0) {
        EXPECT_EQ(comp->getName(), "the minor");
    }
}

TEST(CompositionDependencyTest, FullDependency) {
}