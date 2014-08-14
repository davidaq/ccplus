#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(gaussian) {
    //int kwidth = (int) parameters[0];   
    //int kheight = (int) parameters[1];   
    //float sigma = parameters[2];
    //
    //if (kwidth % 2 != 1 || kheight % 2 != 1)
    //    return;
    //Mat& input = frame.getImage();
    //GaussianBlur(input, input, {kwidth, kheight}, sigma);
    if (parameters.size() == 0)
        return;
    if (parameters.size() < 2) {
        log(CCPlus::logERROR) << "In sufficient parameters for gaussian filter";
        return;
    }
    int blurness = (int) parameters[0];
    int direction = (int) parameters[1];
    
    int kwidth = blurness;
    int kheight = blurness;
    Mat& input = frame.getImage();
    if (kwidth % 2 != 1) kwidth += 1;
    if (kheight % 2 != 1) kheight += 1;
    if (direction == 2) {
        kheight = 1;
    } else if (direction == 3) {
        kwidth = 1;
    }
    GaussianBlur(input, input, {kwidth, kheight}, 0, 0);
}
