#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(volume) {
    if (parameters.size() == 0)
        return;
    // No audio
    if (frame.getAudio().total() == 0) 
        return;

    float perct = (float) parameters[0];
    
    if (perct > 1.01 || perct < 0)
        return;

    for (int i = 0; i < frame.getAudio().total(); i++) {
        short tmp = frame.getAudio().at<short>(i);
        tmp = (short) tmp * perct;
        frame.getAudio().at<short>(i) = tmp;
    }
}
