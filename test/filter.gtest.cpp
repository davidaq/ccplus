#include "gtest/gtest.h" 

#include "filter.hpp"
#include "utils.hpp"

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
    Filter("transform").apply(img1, {50, 100, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0}, 500, 500);
    EXPECT_TRUE(std::equal(
                original.begin<uchar>(), 
                original.end<uchar>(), 
                img1.getImage()(
                    Range(100, 100 + height), 
                    Range(50, 50 + width)).begin<uchar>()));

    // Test anchor + position
    Filter("transform").apply(img2, {0, 0, 0, 50, 10, 0, 1.0, 1.0, 1.0, 0, 0, 0}, 500, 500);
    img2.write("tmp/wtf2.jpg");
    Mat sample = original(Range(10, original.rows - 10), Range(50, original.cols - 50));
    Mat result = img2.getImage()(Range(10, original.rows - 10), Range(50, original.cols - 50));
}

TEST(Filter, ScaleAndRotateTransform) {
    printf("Go check the tmp dir\n");
    
    Frame img1("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 45}, 500, 500);
    img1.write("tmp/rotateCW45.jpg");

    img1 = Frame("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 90}, 500, 500);
    img1.write("tmp/rotateCW90_1.jpg");

    img1 = Frame("test/res/test1.jpg");
    Filter("transform").apply(img1, {243, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 90}, 500, 500);
    img1.write("tmp/rotateCW90_2.jpg");

    img1 = Frame("test/res/test1.jpg");
    Filter("transform").apply(img1, {250, 250, 0, 140, 122, 0, 1.5, 1.5, 1.0, 0, 0, 90}, 500, 500);
    img1.write("tmp/center_and_scale.jpg");
}

TEST(Filter, 3DTransform) {
    for (int i = 0; i <= 180; i += 10) {
        Frame img1("test/res/test1.jpg");
        Filter("transform").apply(img1, {250, 250, 0, 140, 122, 0, 1.5, 1.5, 1.0, 0, (float)i, 90}, 500, 500);
        img1.write("tmp/3dRot_y" + toString(i) + ".jpg");
    }
    for (int i = 0; i <= 180; i += 10) {
        Frame img1("test/res/test1.jpg");
        Filter("transform").apply(img1, {250, 250, 0, 140, 122, 0, 1.5, 1.5, 1.0, (float)i, 0, 0}, 500, 500);
        img1.write("tmp/3dRot_x" + toString(i) + ".jpg");
    }
}

TEST(Filter, MaskTest) {
    printf("Go check the tmp dir\n");
    
    Frame img1("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 0, 0, 2.0, 2.0, 1.0, 0, 0, 0}, 500, 500);
    Filter("mask").apply(img1, {50, 300, 250, 100, 450, 300, 250, 500}, 500, 500);
    img1.write("tmp/mask.png");
}

TEST(Filter, HSLTest) {
    for (int i = 0 ; i <= 10; i++) {
        Frame img1("test/res/test2.jpg");
        Filter("hsl").apply(img1, {135, 1.1, 1.0f + i / 10.0f}, 500, 500);
        img1.write("tmp/hsl" + toString(i) + ".png");
    }
}

TEST(Filter, GrayScaleTest) {
    Frame img1(Mat(5, 5, CV_8UC4, {0, 255, 127, 127}));
    Filter("grayscale").apply(img1, {40, 60, 40, 60, 20, 80, 0, 0}, 30, 30);
    Mat mat = img1.getImage();
    for (int i = 0; i < mat.cols; i++)
        for (int j = 0; j < mat.rows; j++)
            EXPECT_EQ(mat.at<Vec4b>(i, j), Vec4b(127, 127, 127, 127));
    img1.write("tmp/mask.png");
}
