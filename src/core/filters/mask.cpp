#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(mask) {
    int pointNum = parameters.size() / 2;
    std::vector<cv::Point> points(pointNum);
    cv::Point* ptr = &points[pointNum - 1];
    for(int i = 0; i < pointNum; i++) {
        *(ptr--) = (cv::Point((int)(parameters[(i << 1) + 1] * 8),
            (int)(parameters[i << 1] * 8)));
    }
    
    const cv::Point *contours[1] = {&points[0]};
    cv::Mat maskImg(frame.getHeight(), frame.getWidth(), CV_8UC1, cv::Scalar(0));
    cv::fillPoly(maskImg, contours, &pointNum, 1, cv::Scalar(255), 8, 3);
    unsigned char* srcPtr = frame.getImage().data;
    unsigned char* mskPtr = maskImg.data;
    for(int i = 0, j = 3, c = maskImg.total(); i < c; i++, j+=4) {
        srcPtr[j] = ((int)srcPtr[j] * mskPtr[i]) / 255;
    }
}

