#include "filter.hpp"
#include "utils.hpp"
#include "logger.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(hsl) {
    if (parameters.size() < 3) return;

    float hue = parameters[0]; // 0 ~ 180
    float sat = parameters[1]; // 0 ~ 2.0
    float lit = parameters[2]; // 0 ~ 2.0
    
    Mat &img = frame.getImage();
    Mat buf;
    cvtColor(img, buf, CV_BGRA2BGR);
    cvtColor(buf, buf, CV_BGR2HLS);
    for(int y = 0; y < img.rows; y++) {
        for(int x = 0; x < img.cols; x++) {
            Vec3b &pixel = buf.at<Vec3b>(y, x);
            int p = pixel[0];
            p += hue;
            while(p >= 180)
                p -= 180;
            while(p < 0)
                p += 180;
            pixel[0] = p;

            // L
            float now_lit = pixel[1];
            float diff = (lit - 1.0) * (lit > 1.0 ? (255 - now_lit) : now_lit);
            float ret = between<float>(now_lit + diff, 0.0, 255.0);
            pixel[1] = (unsigned char) ret;

            // S
            float now_sat = pixel[2];
            diff = (sat - 1.0) * (sat > 1.0 ? (255 - now_sat) : now_sat);
            ret = between<float>(now_sat + diff, 0.0, 255.0);
            pixel[2] = (unsigned char) ret;
        }
    }
    cvtColor(buf, buf, CV_HLS2BGR);
    for(int y = 0; y < img.rows; y++) {
        for(int x = 0; x < img.cols; x++) {
            Vec4b &opx = img.at<Vec4b>(y, x);
            Vec3b &bpx = buf.at<Vec3b>(y, x);
            for(int i = 0; i < 3; i++) {
                opx[i] = bpx[i];
            }
        }
    }
}

