#include "filter.hpp"
#include "gpu-frame.hpp"
#include "gpu-double-buffer.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(volume) {
    if (parameters.size() == 0)
        return;
    // No audio
    if (frame.ext.audio.total() == 0) 
        return;

    float perct = (float) parameters[0]; 
    if (perct > 1.01 || perct < 0) return;

    for (int i = 0; i < frame.ext.audio.total(); i++) {
        short tmp = frame.ext.audio.at<short>(i);
        tmp = (short) tmp * perct;
        frame.ext.audio.at<short>(i) = tmp;
    }
    GPUDoubleBuffer::preventSwap();
}
