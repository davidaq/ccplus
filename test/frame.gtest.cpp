#include <gtest/gtest.h>
#include "global.hpp"
#include "ios"
#include <opencv2/opencv.hpp>

using namespace CCPlus;
using namespace cv;

TEST(Frame, FrameReadTest) {
    Frame img("test/res/test.png");
    EXPECT_EQ(img.getHeight(), 1080);
    EXPECT_EQ(img.getWidth(), 1920);
}

TEST(Frame, OverlayFrameTest1) {
    Frame img1("test/res/test_alpha.png");
    Frame img2("test/res/test_alpha.png");
    Frame img3("test/res/test.png");
    Frame img4("test/res/test.png");
}

TEST(Frame, OverlayFrameTest2) {
    Mat m1 = Mat::zeros(2, 2, CV_8UC4);
    Mat m2(2, 2, CV_8UC4, {255, 255, 255, 127});

    m1.at<Vec4b>(1, 0) = {160, 200, 0, 256 / 4 - 1};

    Frame img1, img2;
    img1.setImage(m1);
    img2.setImage(m2);

    img2.mergeFrame(img1);
    
    EXPECT_EQ(img2.getImage().at<Vec4b>(0, 0)[0], 255);
    EXPECT_LE(std::abs(img2.getImage().at<Vec4b>(1, 0)[0]) - 255, 1);
    EXPECT_LE(std::abs(img2.getImage().at<Vec4b>(1, 0)[1]) - 255, 1);
}

TEST(Frame, trackMatteTest) {
    Mat m1 = Mat::zeros(2, 2, CV_8UC4);
    m1.at<Vec4b>(1, 1)[3] = 255;
    m1.at<Vec4b>(0, 1)[3] = 127;
    Mat m2(2, 2, CV_8UC4, {50, 100, 150, 127});
    Frame f1(m1);
    Frame f2(m2);
    f2.trackMatte(f1, 1);
    EXPECT_EQ(0, f2.getImage().at<Vec4b>(0, 0)[3]);
    EXPECT_EQ(127, f2.getImage().at<Vec4b>(1, 1)[3]);
    EXPECT_EQ(63, f2.getImage().at<Vec4b>(0, 1)[3]);
}

TEST(Frame, EmtpyFrameTest) {
    Frame img = Frame::emptyFrame(2, 3);
    EXPECT_EQ(img.getWidth(), 2);
    EXPECT_EQ(img.getHeight(), 3);
} 

TEST(Frame, To4ChannelsTest) {
    Frame img("test/res/test.png");
    Mat mat = imread("test/res/test.png", CV_LOAD_IMAGE_UNCHANGED);

    // Pick some random pixel to make sure 4th channel is presented
    EXPECT_EQ(img.getHeight(), mat.rows);
    EXPECT_EQ(img.getWidth(), mat.cols);
    EXPECT_EQ(img.getImage().channels(), 4);
    EXPECT_EQ(img.getImage().at<Vec4b>(100, 200)[3], 255);
    EXPECT_EQ(img.getImage().at<Vec4b>(600, 800)[3], 255);

    // Pick some random pixels to make sure color is the same
    EXPECT_EQ(img.getImage().at<Vec4b>(1024, 800)[2], mat.at<Vec3b>(1024, 800)[2]);
    EXPECT_EQ(img.getImage().at<Vec4b>(256, 3)[2], mat.at<Vec3b>(256, 3)[2]);
    EXPECT_EQ(img.getImage().at<Vec4b>(0, 0)[0], mat.at<Vec3b>(0, 0)[0]);
}

TEST(Frame, FrameRotateTest) {
    Frame img("test/res/test3.jpg");
    Mat mat = imread("test/res/test3.jpg", CV_LOAD_IMAGE_UNCHANGED);

    EXPECT_EQ(mat.rows, img.getWidth());
    EXPECT_EQ(mat.cols, img.getHeight());

    EXPECT_EQ(mat.at<Vec3b>(0, 0)[0], 
            img.getImage().at<Vec4b>(img.getHeight() - 1, 0)[0]);
    EXPECT_EQ(mat.at<Vec3b>(0, 0)[1], 
            img.getImage().at<Vec4b>(img.getHeight() - 1, 0)[1]);
    EXPECT_EQ(mat.at<Vec3b>(0, 0)[2], 
            img.getImage().at<Vec4b>(img.getHeight() - 1, 0)[2]);
    
    //img.write("test/res/test3_result.jpg");
}

TEST(Frame, CompressAndDecompress) {
    Frame img("test/res/test_alpha.png");
    
    img.write("tmp/compress-test.zim");
    Frame img2("tmp/compress-test.zim");
    img2.write("tmp/decompress-test.png");
    
    EXPECT_EQ(img.getWidth(), img2.getWidth());
    EXPECT_EQ(img.getHeight(), img2.getHeight());
    EXPECT_EQ(img.getAudio().total(), 0);
    EXPECT_EQ(img2.getAudio().total(), 0);
}

TEST(Frame, CompressAndDecompressAudio) {
    Frame frame("test/res/test_alpha.png");
    std::vector<int16_t> tmp;
    for (int i = 0; i < 1333; i++)
        tmp.push_back(i % 10);
    frame.setAudio(tmp);
    
    frame.write("tmp/compress-test.zim");
    Frame img2("tmp/compress-test.zim");
    
    EXPECT_EQ(frame.getWidth(), img2.getWidth());
    EXPECT_EQ(frame.getHeight(), img2.getHeight());
    EXPECT_EQ(frame.getAudio().total(), 1333);
    EXPECT_EQ(img2.getAudio().total(), 1333);

    if (img2.getAudio().total() == 1333)
        for (int i = 0; i < 1333; i++) 
            EXPECT_EQ(img2.getAudio().at<int16_t>(i), i % 10);
}

