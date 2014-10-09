#include "filter.hpp"
#include "logger.hpp"
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace CCPlus;

const std::vector<float> HUE = {0, 30, 60, 90, 120, 150, 180};

CCPLUS_FILTER(grayscale) {
    if (parameters.size() < 5)
        return;
    /*
     * Parameter 0~5: From -200 ~ 300, unit: %
     * Parameter 6: 0 ~ 360, unit: %
     * Parameter 7: 0 ~ 1, unit: %
     */

    Mat& mat = frame.getImage();
    std::vector<unsigned char> alphas;
    for (int i = 3, j = 0; j < mat.total(); j++, i += 4) 
        alphas.push_back(mat.data[i]);
    cvtColor(mat, mat, CV_BGR2HSV);

    for (int i = 0; i < mat.cols; i++) {
        for (int j = 0; j < mat.rows; j++) {
            Vec3b& hsv = mat.at<Vec3b>(i, j);
            //L() << hsv;
            float h = hsv[0];
            float v = (float) hsv[2];
            float ret;
            int idx = int(h) / 30;
            if (int(h) % 30 == 0) {
                ret = (unsigned char) parameters[idx] / 100.0 * v;
            } else {
                ret = (h - HUE[idx]) / 30.0 * 
                    parameters[idx] / 100.0 * v;
                //L() << h << " " << idx << " " << ret;
                ret += (HUE[idx + 1] - h) / 30.0 * 
                    parameters[(idx + 1) % 6] / 100.0 * v;
            }
            ret = std::max(0.0f, ret);
            ret = std::min(255.0f, ret);

            hsv[0] = (unsigned char) parameters[6] / 2;
            hsv[1] = (unsigned char) parameters[7] * 255.0;
            hsv[2] = (unsigned char) ret;
            //L() << hsv;
        }
    }
    
    cvtColor(mat, mat, CV_HSV2BGR);
    //frame.addAlpha(alphas);
    return;
}
