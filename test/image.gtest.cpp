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

TEST(Image, OverlayImageTest) {
    Image img1("test/res/test_alpha.png");
    Image img2("test/res/test_alpha.png");
    Image img3("test/res/test.png");
    Image img4("test/res/test.png");
    EXPECT_NO_THROW(img1.overlayImage(&img2));
    EXPECT_THROW(img1.overlayImage(&img3), std::invalid_argument);
    EXPECT_THROW(img3.overlayImage(&img1), std::invalid_argument);

    EXPECT_EQ(img1.getData()->at<Vec4b>(100, 200)[0], img2.getData()->at<Vec4b>(100, 200)[0]);
}
