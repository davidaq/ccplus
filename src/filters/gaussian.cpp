#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(gaussian) {
    int kwidth = (int) parameters[0];   
    int kheight = (int) parameters[1];   
    float sigma = parameters[2];
    
    if (kwidth % 2 != 1 || kheight % 2 != 1)
        return;
    Mat& input = frame.getImage();
    GaussianBlur(input, input, {kwidth, kheight}, sigma);
}
