#include <gtest/gtest.h>
#include "global.hpp"
#include "ios"
#include <opencv2/opencv.hpp>

using namespace CCPlus;
using namespace cv;

TEST(Image, ImageReadTest) {
    Image img("test/res/test.png");
    EXPECT_EQ(img.getHeight(), 1080);
    EXPECT_EQ(img.getWidth(), 1920);

    EXPECT_THROW(Image("hithere"), std::ios_base::failure);
}

TEST(Image, OverlayImageTest1) {
    Image img1("test/res/test_alpha.png");
    Image img2("test/res/test_alpha.png");
    Image img3("test/res/test.png");
    Image img4("test/res/test.png");
    //EXPECT_NO_THROW(img1.overlayImage(&img2));
    EXPECT_THROW(img1.overlayImage(&img3), std::invalid_argument);
    EXPECT_THROW(img3.overlayImage(&img1), std::invalid_argument);
}

TEST(Image, OverlayImageTest2) {
    Mat m1 = Mat::zeros(2, 2, CV_8UC4);
    Mat m2(2, 2, CV_8UC4, {255, 255, 255, 127});

    m1.at<Vec4b>(1, 0) = {160, 200, 0, 256 / 4 - 1};

    Image img1, img2;
    img1.setData(m1);
    img2.setData(m2);

    img2.overlayImage(&img1);
    
    EXPECT_EQ(img2.getData()->at<Vec4b>(0, 0)[0], 127);
    EXPECT_LE(std::abs(img2.getData()->at<Vec4b>(1, 0)[0]) - 147, 1);
    EXPECT_LE(std::abs(img2.getData()->at<Vec4b>(1, 0)[1]) - 152, 1);
    //EXPECT_EQ(img2.at<Vec4b>(0, 1)[2], 127);
    //EXPECT_EQ(img2.at<Vec4b>(0, 1)[2], 127);
    //EXPECT_EQ(img1.getData()->at<Vec4b>(100, 200)[0], img2.getData()->at<Vec4b>(100, 200)[0]);
}
