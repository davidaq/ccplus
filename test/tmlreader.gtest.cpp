#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

class TMLReaderTest : public ::testing::Test {

protected:
    virtual void SetUp() {
        ctx = new Context("res/tmp");
        this->reader = new TMLReader(ctx);
        ctx->retain(this->reader);
        ASSERT_NO_THROW(this->mainComp = reader->read("test/res/test1.tml"));
    }

    virtual void TearDown() {
        delete ctx;
    }

    Composition* mainComp;
    TMLReader* reader;
    Context* ctx;
};

//TEST_F(TMLReaderTest, IOTest) {
//    ASSERT_NO_THROW(this->mainComp = reader->read("test/res/test1.tml"));
//}

TEST_F(TMLReaderTest, MainCompBasicTest) {
    EXPECT_EQ(mainComp->getName(), "MAIN");
    EXPECT_EQ(mainComp->getDuration(), 10);
}

TEST_F(TMLReaderTest, MainCompSizeTest) {
    EXPECT_EQ(mainComp->getWidth(), 1920);
    EXPECT_EQ(mainComp->getHeight(), 1080);
}

TEST_F(TMLReaderTest, SubCompTest) {
    std::string name = std::string("") + "composition://" + "the minor";
    //std::cout << name << std::endl;
    //std::cout << ctx->numberOfRenderable() << std::endl;
    ASSERT_TRUE(ctx->hasRenderable(name));
    EXPECT_EQ(ctx->getRenderable(name)->getDuration(), 20);
    EXPECT_EQ(ctx->getRenderable(name)->getWidth(), 100);
    EXPECT_EQ(ctx->getRenderable(name)->getHeight(), 200);
}

TEST_F(TMLReaderTest, LayersTest) {
    std::vector<Layer> layers = mainComp->getLayers();
    Layer l = layers[0];
    EXPECT_EQ(l.getTime(), 1);       
    EXPECT_EQ(l.getDuration(), 10);       
    EXPECT_EQ(l.getStart(), 2);       
    EXPECT_EQ(l.getLast(), 11);       
}

