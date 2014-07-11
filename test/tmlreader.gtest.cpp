#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

class TMLReaderTest : public ::testing::Test {
 public:
  TMLReaderTest() : ctx("res/tmp") {};
 protected:
  virtual void SetUp() {
    TMLReader reader(ctx);
    this->mainComp = reader.read("res/test1.tml");
  }

  // virtual void TearDown() {}

  Context ctx;
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

TEST_F(TMLReaderTest, LayersTest) {
    EXPECT_EQ(0, 0);       
}
