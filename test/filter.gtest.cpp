#include "gtest/gtest.h" 

#include "filter.hpp"

using namespace CCPlus;
using namespace cv;

TEST(Filter, Example) {
    // This test currently asserts nothing, passes if no segment fault occurs
    printf("This test should print PASS on the next line\n");
    std::vector<float> empty;
    Image src("test/res/test.png"), dest("test/res/test.png");
    Filter("Example").apply(&src, &dest, empty);
}

TEST(Filter, Transform) {
    Image src("test/res/test1.jpg");
    int width = src.getWidth();
    int height = src.getHeight();
    Image dest = Image::emptyImage(500, 500);
    Filter("transform").apply(&src, &dest, {100, 50, 0, 0, 0, 0, 0});

    //dest.write("test/res/p.jpg");

    EXPECT_TRUE(std::equal(
                src.getData().begin<uchar>(), 
                src.getData().end<uchar>(), 
                dest.getData()(
                    Range(100, 100 + height), 
                    Range(50, 50 + width)).begin<uchar>()));
}
