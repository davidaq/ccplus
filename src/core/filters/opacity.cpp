#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(opacity) {
    float opa = (float) parameters[0];

    if (opa > 1.00001 && opa < 0.0) {
        printf("Invalid opacity argument");
        return;
    }

    Mat& img = frame.getImage();
    for (int i = 0; i < img.rows; i++)
        for (int j = 0; j < img.cols; j++)
            img.at<Vec4b>(i, j)[3] = (unsigned char)(255.0 * opa);
}

