#include "gtest/gtest.h" 

#include "tmlreader.hpp"
#include "composition.hpp"

using namespace CCPlus;

class TMLReaderTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
      this->mainComp = TMLReader::read("res/test1.tml");
  }

  // virtual void TearDown() {}

  Composition mainComp;
};

TEST_F(TMLReaderTest, MainCompBasicTest) {
    EXPECT_EQ(mainComp.getName(), "MAIN");
    EXPECT_EQ(mainComp.getVersion(), 0.0);
    EXPECT_EQ(mainComp.getDuration(), 0);
}

TEST_F(TMLReaderTest, MainCompSizeTest) {
    EXPECT_EQ(mainComp.getWidth(), 1920);
    EXPECT_EQ(mainComp.getHeight(), 1080);
}

