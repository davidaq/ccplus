#include "gtest/gtest.h" 
#include "tmlreader.hpp"
#include "composition.hpp"

TEST(tmlreadertest, boringtest)  {
    EXPECT_EQ(1, 1);
}

TEST(TMLReaderTest, BasicTest) {
    Composition comp = TMLReader::read("yo");
    EXPECT_EQ(comp.getName(), "lovely heck");
}
