#include "gtest/gtest.h" 

#include "filter.hpp"
#include "utils.hpp"

#include <vector>

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
    
    // Test position
    Filter("transform").apply(img1, {50, 100, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0}, 500, 500);
    img1.write("tmp/wtf1.jpg");

    // Test anchor + position
    Filter("transform").apply(img2, {0, 0, 0, 50, 10, 0, 1.0, 1.0, 1.0, 0, 0, 0}, 500, 500);
    img2.write("tmp/wtf2.jpg");
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
        Filter("transform").apply(img1, {250, 250, 0, 140, 122, 0, 1.5, 1.5, 1.0, (float)i, 0, 90}, 500, 500);
        img1.write("tmp/3dRot_x" + toString(i) + ".jpg");
    }
}

TEST(Filter, MaskTest) {
    printf("Go check the tmp dir\n");
    
    Frame img1("test/res/test1.jpg");
    Filter("transform").apply(img1, {0, 0, 0, 0, 0, 0, 2.0, 2.0, 1.0, 0, 0, 0}, 500, 500);
    Filter("mask").apply(img1, {50, 50, 50, 300, 250, 100, 450, 300, 250, 500}, 500, 500);
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

TEST(Filter, RampTest) {
    Frame img("test/res/test2.jpg");
    Filter("ramp").apply(img, {1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852);
    img.write("tmp/ramp_radial.jpg");

    Frame img2("test/res/test2.jpg");
    Filter("ramp").apply(img2, {-1, 300, 0, 0, 0, 0, 300, 800, 255, 255, 255, 0.2}, 640, 852);
    img2.write("tmp/ramp_linear.jpg");
}

TEST(Filter, 4ColorTest) {
    // None
    Frame img("test/res/test2.jpg");
    std::vector<float> parameters = {
        100, 100,               255, 0, 0, // Blue
        639 - 50, 0 + 70,       0, 255, 0, // Green
        0 + 110, 851 - 200,     0, 0, 255, // Red
        639 - 60, 851 - 150,    0, 255, 255, // yellow
        5, 0.5, -1 // none
    };
    Filter("4color").apply(img, parameters, 640, 852);
    img.write("tmp/4color_none.jpg");

    img = Frame("test/res/test2.jpg");
    parameters = {
        0, 0, 255, 0, 0, // Blue
        639, 0, 0, 255, 0, // Green
        0, 851, 0, 0, 255, // Red
        639, 851, 255, 255, 255, // White
        5, 0.5, 0
    };
    Filter("4color").apply(img, parameters, 640, 852);
    img.write("tmp/4color_normal.jpg");

    // Add Blend
    img = Frame("test/res/test2.jpg");
    parameters = {
        0, 0, 255, 0, 0, // Blue
        639, 0, 0, 255, 0, // Green
        0, 851, 0, 0, 255, // Red
        639, 851, 255, 255, 255, // White
        5, 0.5, 1 // Add
    };
    Filter("4color").apply(img, parameters, 640, 852);
    img.write("tmp/4color_add.jpg");

    // Multiply Blend
    img = Frame("test/res/test2.jpg");
    parameters = {
        0, 0, 255, 0, 0, // Blue
        639, 0, 0, 255, 0, // Green
        0, 851, 0, 0, 255, // Red
        639, 851, 255, 255, 255, // White
        5, 0.5, 2 // multiply
    };
    Filter("4color").apply(img, parameters, 640, 852);
    img.write("tmp/4color_multiply.jpg");

    // Screen
    img = Frame("test/res/test2.jpg");
    parameters = {
        0, 0, 255, 0, 0, // Blue
        639, 0, 0, 255, 0, // Green
        0, 851, 0, 0, 255, // Red
        639, 851, 0, 255, 255, // White
        5, 0.5, 3 // none
    };
    Filter("4color").apply(img, parameters, 640, 852);
    img.write("tmp/4color_screen.jpg");
}

TEST(Filter, CurveTest) {
    Frame img = Frame("test/res/test1.jpg");
    std::vector<float> parameters = {0, 255, 0, 0, 0, 255, 0, 255, 127, 127};
    Filter("curve").apply(img, parameters, 640, 852);
    img.write("tmp/curve_1.jpg");
}

TEST(Filter, LensFlareTest) {
    Frame img = Frame("test/res/test2.jpg");
    std::vector<float> parameters = {160, 150, 1, 0};
    Filter("lens_flare").apply(img, parameters, 500, 500);
    img.write("tmp/flare.jpg");
}
