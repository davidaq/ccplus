#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(opacity) {
    if (parameters.size() == 0)
        return;
    float opa = (float) parameters[0];

    if (opa > 1.00001 && opa < 0.0) {
        printf("Invalid opacity argument");
        return;
    }

    Mat& img = frame.getImage();
    for (int i = 0; i < img.rows; i++) {
        Vec4b* ptr = img.ptr<Vec4b>(i);
        for (int j = 0; j < img.cols; j++) {
            //img.at<Vec4b>(i, j)[3] = (unsigned char)(opa * img.at<Vec4b>(i, j)[3]);
            ptr[j][3] = (unsigned char)(opa * ptr[j][3]);
        }
    }
}

