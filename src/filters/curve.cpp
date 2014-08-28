#include "filter.hpp"
#include "logger.hpp"

#include "frame.hpp"

#include <cmath>
#include <vector>

using namespace CCPlus;
using namespace cv;


/*
 * 4 curves: alpha, blue, green, red
 */
CCPLUS_FILTER(curve) {
    if (parameters.size() == 0)
        return;
    if (parameters.size() % 5 != 0 || parameters.size() < 5) {
        log(CCPlus::logERROR) << "Invalid parameters for curve effect";
        return;
    }

    int n = parameters.size() / 5;

    auto calcIdx = [n] (unsigned char v) -> int {
        // return from 0 ~ n
        return (int) 1.0f * (n - 1) * v / 255.0;
    };

    for (int i = 0; i < frame.getWidth(); i++)
        for (int j = 0; j < frame.getHeight(); j++) {
            for (int k = 0; k < 3; k++) {
                // TODO: make sure which curve been applied first
                int idx = calcIdx(frame.getImage().at<Vec4b>(j, i)[k]);
                // Go through r/g/b curve
                unsigned char tmp = (unsigned char) parameters[(1 + k) * n + idx]; 
                idx = calcIdx(tmp);
                // Go through rgb curve
                frame.getImage().at<Vec4b>(j, i)[k] = (unsigned char) parameters[idx];
            }
            frame.getImage().at<Vec4b>(j, i)[3] = (unsigned char)
                parameters[calcIdx(frame.getImage().at<Vec4b>(j, i)[3]) + 4 * n];
        }
}
