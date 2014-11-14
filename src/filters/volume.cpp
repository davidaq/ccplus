#include "filter.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(volume) {
    if (parameters.size() == 0) {
        log(logERROR) << "Not enough parameters for volume";
        return frame;
    }
    // No audio
    if (frame->ext.audio.empty())
        return frame;

    float perct = (float) parameters[0]; 
    if(10 == (int)(perct * 10)) return frame;
    if(perct > 2)
        perct = 2;
    if(perct < 0)
        perct = 0;

    for (int i = 0; i < frame->ext.audio.total(); i++) {
        int tmp = frame->ext.audio.at<short>(i);
        tmp = (int) tmp * perct;
        if(tmp > 32767)
            tmp = 32767;
        if(tmp < -32768)
            tmp = -32768;
        frame->ext.audio.at<short>(i) = tmp;
    }
    return frame;
}
