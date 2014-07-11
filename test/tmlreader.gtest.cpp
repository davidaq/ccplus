#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

class TMLReaderTest : public ::testing::Test {
<<<<<<< HEAD
 public:
  TMLReaderTest() : ctx("res/tmp") {};
 protected:
  virtual void SetUp() {
    TMLReader reader(ctx);
    this->mainComp = reader.read("res/test1.tml");
  }
=======
>>>>>>> 370fe4f0cb636e693f542a657a26f4f900ee7d3c

protected:
    virtual void SetUp() {
        Context ctx("res/tmp");
        this->reader = new TMLReader(ctx);
    }

<<<<<<< HEAD
  Context ctx;
  Composition mainComp;
=======
    virtual void TearDown() {
        delete reader;
    }

    Composition mainComp;
    TMLReader* reader;
>>>>>>> 370fe4f0cb636e693f542a657a26f4f900ee7d3c
};

TEST_F(TMLReaderTest, IOTest) {
    ASSERT_NO_THROW(this->mainComp = reader->read("test/res/test1.tml"));
}

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
