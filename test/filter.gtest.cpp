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
    Image img1("test/res/test1.jpg");
    Image img2("test/res/test1.jpg");
    
    cv::Mat original = img1.getData().clone();
    
    int width = img1.getWidth();
    int height = img1.getHeight();
    
    // Test position
    Filter("transform").apply(img1, {100, 50, 0, 0, 0, 0, 0}, 500, 500);
    EXPECT_TRUE(std::equal(
                original.begin<uchar>(), 
                original.end<uchar>(), 
                img1.getData()(
                    Range(100, 100 + height), 
                    Range(50, 50 + width)).begin<uchar>()));

    // Test anchor + position
    Filter("transform").apply(img2, {0, 0, 10, 50, 0, 0, 0}, 500, 500);
    Mat sample = original(Range(10, original.rows - 10), Range(50, original.cols - 50));
    Mat result = img1.getData()(Range(10, original.rows - 10), Range(50, original.cols - 50));
    EXPECT_TRUE(std::equal(original.begin<uchar>(), 
                original.begin<uchar>(),
                result.begin<uchar>()));

    // TODO more test cases
}
