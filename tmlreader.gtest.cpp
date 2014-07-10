#include "gtest/gtest.h" 
#include "tmlreader.hpp"
#include "composition.hpp"

TEST(demotest, boringtest)  {
    EXPECT_EQ(1, 0);
}

TEST(TMLReaderTest, BasicTest) {
    Composition comp = TMLReader.read();
    EXPECT_EQ(comp.getName(), "lovely heck!");
}
