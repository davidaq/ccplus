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
}

TEST(Image, EmtpyImageTest) {
    Image img = Image::emptyImage(2, 3);
    EXPECT_EQ(img.getWidth(), 2);
    EXPECT_EQ(img.getHeight(), 3);
} 

TEST(Image, To4ChannelsTest) {
    Image img("test/res/test.png");
    Mat mat = imread("test/res/test.png", CV_LOAD_IMAGE_UNCHANGED);

    // Pick some random pixel to make sure 4th channel is presented
    EXPECT_EQ(img.getHeight(), mat.rows);
    EXPECT_EQ(img.getWidth(), mat.cols);
    EXPECT_EQ(img.getData()->channels(), 4);
    EXPECT_EQ(img.getData()->at<Vec4b>(100, 200)[3], 255);
    EXPECT_EQ(img.getData()->at<Vec4b>(600, 800)[3], 255);

    // Pick some random pixels to make sure color is the same
    EXPECT_EQ(img.getData()->at<Vec4b>(1024, 800)[2], mat.at<Vec3b>(1024, 800)[2]);
    EXPECT_EQ(img.getData()->at<Vec4b>(256, 3)[2], mat.at<Vec3b>(256, 3)[2]);
    EXPECT_EQ(img.getData()->at<Vec4b>(0, 0)[0], mat.at<Vec3b>(0, 0)[0]);
}

TEST(Image, CompressAndDecompress) {
    Image img("test/res/test.png");
    
    img.write("test/res/compress-test.zim");
    Image img2("test/res/compress-test.zim");
    
    remove("test/res/compress-test.zim");
    
    EXPECT_EQ(img.getWidth(), img2.getWidth());
    EXPECT_EQ(img.getHeight(), img2.getHeight());
    EXPECT_EQ(img.getData()->data[5], img2.getData()->data[5]);
}
