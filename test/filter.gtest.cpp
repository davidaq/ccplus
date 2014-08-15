#include "gtest/gtest.h" 

#include "filter.hpp"

using namespace CCPlus;
using namespace cv;

TEST(Filter, Example) {
    // This test currently asserts nothing, passes if no segment fault occurs
    printf("This test should print PASS on the next line\n");
    std::vector<float> empty;
    Frame src("test/res/test.png"), dest("test/res/test.png");
    Filter("Example").apply(src, empty, src.getWidth(), src.getHeight());
}

TEST(Filter, BasicTransform) {
    Frame img1("test/res/test1.jpg");
    Frame img2("test/res/test1.jpg");
    
    cv::Mat original = img1.getImage().clone();
    
    int width = img1.getWidth();
    int height = img1.getHeight();
    
    // Test position
    Filter("transform").apply(img1, {100, 50, 0, 0, 1.0, 1.0, 0}, 500, 500);
    EXPECT_TRUE(std::equal(
                original.begin<uchar>(), 
                original.end<uchar>(), 
                img1.getImage()(
                    Range(100, 100 + height), 
                    Range(50, 50 + width)).begin<uchar>()));

    // Test anchor + position
    Filter("transform").apply(img2, {0, 0, 10, 50, 1.0, 1.0, 0}, 500, 500);
    img2.write("tmp/wtf2.jpg");
    Mat sample = original(Range(10, original.rows - 10), Range(50, original.cols - 50));
    Mat result = img2.getImage()(Range(10, original.rows - 10), Range(50, original.cols - 50));
    // There is interpolation, so no longer works
    //EXPECT_TRUE(std::equal(original.begin<uchar>(), 
    //            original.end<uchar>(),
    //            result.begin<uchar>()));
}

TEST(Filter, ScaleAndRotateTransform) {
    printf("Go check the tmp dir\n");
    
    Frame img1("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 1.0, 1.0, 45}, 500, 500);
    img1.write("tmp/rotateCW45.jpg");

    img1 = Frame("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 1.0, 1.0, 90}, 500, 500);
    img1.write("tmp/rotateCW90_1.jpg");

    img1 = Frame("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 243, 0, 0, 1.0, 1.0, 90}, 500, 500);
    img1.write("tmp/rotateCW90_2.jpg");

    img1 = Frame("test/res/test1.jpg");
    //Filter("transform").apply(img1, {250, 250, 122, 140, 1.5, 1.5, 0}, 500, 500);
    Filter("transform").apply(img1, {250, 250, 122, 140, 1.5, 1.5, 90}, 500, 500);
    img1.write("tmp/center_and_scale.jpg");
}

TEST(Filter, MaskTest) {
    printf("Go check the tmp dir\n");
    
    Frame img1("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 2.0, 2.0, 0}, 500, 500);
    Filter("mask").apply(img1, {50, 300, 250, 100, 450, 300, 250, 500}, 500, 500);
    img1.write("tmp/mask.png");
}

TEST(Filter, HSLTest) {
    Frame img1("test/res/test2.jpg");
    Filter("hsl").apply(img1, {135, 1.1, 0.9}, 500, 500);
    img1.write("tmp/hsl.png");
}
