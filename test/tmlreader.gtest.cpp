#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

class TMLReaderTest : public ::testing::Test {

protected:
    virtual void SetUp() {
        ctx = new Context("res/tmp", 18);
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
    EXPECT_EQ(((Composition*)ctx->getRenderable(name))->getName(), "the minor");
    EXPECT_EQ(ctx->getRenderable(name)->getDuration(), 20);
    EXPECT_EQ(ctx->getRenderable(name)->getWidth(), 100);
    EXPECT_EQ(ctx->getRenderable(name)->getHeight(), 200);
    
    
    EXPECT_EQ(((Composition*)ctx->getRenderable(name))->getName(), "the minor");
}

TEST_F(TMLReaderTest, LayersTest) {
    std::vector<Layer> layers = mainComp->getLayers();
    Layer l = layers[0];
    EXPECT_EQ(l.getTime(), 1);       
    EXPECT_EQ(l.getDuration(), 10);       
    EXPECT_EQ(l.getStart(), 2);       
    EXPECT_EQ(l.getLast(), 11);       
}

TEST_F(TMLReaderTest, PropertiesPositionTest) {
    std::vector<Layer> layers = mainComp->getLayers();
    Layer l = layers[0];
    std::map<std::string, Property> mp = l.getProperties();

    // Position
    EXPECT_TRUE(mp.find("position") != mp.end());
    EXPECT_TRUE(mp["position"].find(1.0) == mp["position"].end());
    EXPECT_TRUE(mp["position"].find(0.1) == mp["position"].end());
    EXPECT_TRUE(mp["position"].find(1241241241412) == mp["position"].end());
    EXPECT_TRUE(mp["position"].find(0) != mp["position"].end());

    EXPECT_EQ(mp["position"][0].size(), 2);
    EXPECT_EQ(mp["position"][0][0], 100);
    EXPECT_EQ(mp["position"][0][1], 20);
}

TEST_F(TMLReaderTest, PropertiesScaleTest) {
    std::vector<Layer> layers = mainComp->getLayers();
    Layer l = layers[0];
    std::map<std::string, Property> mp = l.getProperties();

    // Position
    EXPECT_TRUE(mp.find("scale") != mp.end());
    // Eps is 1e-5
    EXPECT_TRUE(mp["scale"].find(1.0000001) != mp["scale"].end());
    EXPECT_TRUE(mp["scale"].find(0) != mp["scale"].end());
    EXPECT_TRUE(mp["scale"].find(10.0) != mp["scale"].end());
    EXPECT_TRUE(mp["scale"].find(0.1) == mp["scale"].end());
    EXPECT_TRUE(mp["scale"].find(1241241241412) == mp["scale"].end());

    EXPECT_EQ(mp["scale"][0].size(), 2);
    EXPECT_EQ(mp["scale"][1.0].size(), 2);
    EXPECT_EQ(mp["scale"][10.0].size(), 2);
    EXPECT_LT(std::abs(mp["scale"][1][0] - 0.5), 0.0001);
    EXPECT_LT(std::abs(mp["scale"][1][1] - 0.7), 0.0001);
}

