#include "gtest/gtest.h" 

#include "global.hpp"
#include "video-renderable.hpp"

using namespace CCPlus;

TEST(CompositionDependencyTest, DirectDependencyTest1) {
    Context ctx("", 18);
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

TEST(CompositionDependencyTest, DirectDependencyTest2) {
    Context* ctx = new Context("tmp", 18);
    std::string uri = "file://test/res/test.mp4";
    VideoRenderable* render = new VideoRenderable(ctx, uri);
    ctx->putRenderable(uri, render);
    Layer l(ctx, uri, 0, 10, 0, 10, 500, 500);
    Composition* mainComp = new Composition(ctx, "main", 1, 500, 500);
    mainComp->putLayer(l);
    EXPECT_EQ(1, mainComp->directDependency(0, 1).size());
}

TEST(CompositionDependencyTest, DirectDependencyTest3) {
    Context* ctx = new Context("tmp", 18);
    std::string uri = "file://test/res/test.mp4";
    VideoRenderable* render = new VideoRenderable(ctx, uri);
    render->retain(ctx);
    ctx->putRenderable(uri, render);

    Layer l(ctx, uri, 0, 5, 0, 10, 500, 500);
    Composition* mainComp = new Composition(ctx, "main", 10, 500, 500);
    mainComp->putLayer(l);
    EXPECT_EQ(1, mainComp->directDependency(3, 7).size());
    EXPECT_EQ(6, mainComp->directDependency(3, 7)[0].from);
    EXPECT_EQ(10, mainComp->directDependency(3, 7)[0].to);

    EXPECT_EQ(1, mainComp->directDependency(7, 30).size());
    EXPECT_EQ(0, mainComp->directDependency(7, 30)[0].from);
    EXPECT_EQ(10, mainComp->directDependency(7, 30)[0].to);
}

TEST(CompositionDependencyTest, FullDependency) {
    Context* ctx = new Context("tmp", 18);
    std::string uri = "file://test/res/test.mp4";
    VideoRenderable* render = new VideoRenderable(ctx, uri);
    ctx->putRenderable(uri, render);
    Layer l(ctx, uri, 0, 1, 0, 1, 500, 500);
    Composition* mainComp = new Composition(ctx, "main", 100, 500, 500);

    mainComp->putLayer(l);

    std::vector<CompositionDependency> deps;
    ASSERT_NO_THROW(deps = mainComp->fullOrderedDependency(0, 10));
    EXPECT_EQ(2, deps.size());
    EXPECT_EQ(0, deps[0].from);
    EXPECT_EQ(1, deps[0].to);

    delete ctx;
}
