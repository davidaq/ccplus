#include <opencv2/opencv.hpp>
#include <gtest/gtest.h>

TEST(OpenCV, ImageRead) {
    cv::Mat img = cv::imread("test/res/test.png");
    EXPECT_EQ(img.size[0], 1080);
    EXPECT_EQ(img.size[1], 1920);
}
