#include "gtest/gtest.h" 

#include "filter.hpp"

using namespace CCPlus;
using namespace cv;

TEST(Filter, Example) {
    // This test currently asserts nothing, passes if no segment fault occurs
    printf("This test should print PASS on the next line\n");
    std::vector<float> empty;
    Image src("test/res/test.png"), dest("test/res/test.png");
    Filter("Example").apply(src, empty, src.getWidth(), src.getHeight());
}

TEST(Filter, Transform) {
    Image src("test/res/test1.jpg");
    
    cv::Mat original = src.getData().clone();
    
    
    int width = src.getWidth();
    int height = src.getHeight();
    
    Filter("transform").apply(src, {100, 50, 0, 0, 0, 0, 0}, 500, 500);
                    

    //dest.write("test/res/p.jpg");
    printf("%dx%d\n", width, height);

    EXPECT_TRUE(std::equal(
                original.begin<uchar>(), 
                original.end<uchar>(), 
                src.getData()(
                    Range(100, 100 + height), 
                    Range(50, 50 + width)).begin<uchar>()));
}
