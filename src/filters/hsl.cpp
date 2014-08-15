#include "filter.hpp"
#include "logger.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(hsl) {
    float hue = parameters[0];
    float sat = parameters[1];
    float lit = parameters[2];
    
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
            pixel[1] = std::min<int>(255, (int)pixel[1] * lit);
            pixel[2] = std::min<int>(255, (int)pixel[2] * sat);
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

