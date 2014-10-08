#include "filter.hpp"
#include "logger.hpp"
#include "mat-cache.hpp"
#include <cmath>

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(mask) {
    if (parameters.size() < 2) {
        log(CCPlus::logERROR) << "Not enough parameters for mask"; 
        return;
    }
    int kwidth = parameters[0];
    int kheight = parameters[1];

    HashFactory hash;
    hash << "MASK" << kwidth << kheight;

    int pointNum = parameters.size() / 2 - 1;
    std::vector<cv::Point> points(pointNum);
    cv::Point* ptr = &points[pointNum - 1];
    for(int i = 1; i <= pointNum; i++) {
        int x = parameters[(i << 1) + 1] * 8;
        int y = parameters[(i << 1)] * 8;
        *(ptr--) = cv::Point(x, y);
        hash << x << y;
    }
    hash << frame.getWidth() << frame.getHeight();
    
    const cv::Point *contours[1] = {&points[0]};
    cv::Mat maskImg = MatCache::get(hash.str(), [&frame, &contours, &pointNum, &kwidth, &kheight] {
        cv::Mat maskImg(frame.getHeight(), frame.getWidth(), CV_8UC1, cv::Scalar(0));
        cv::fillPoly(maskImg, contours, &pointNum, 1, cv::Scalar(255), 8, 3);
        /*
         * feather effect
         */
        if (kwidth != 0 && kheight != 0) {
            if(!(kwidth & 1))
                kwidth++;
            if(!(kheight & 1))
                kheight++;
            GaussianBlur(maskImg, maskImg, {kwidth, kheight}, std::max(kwidth, kheight));
            //cv::fillPoly(maskImg, contours, &pointNum, 1, cv::Scalar(255), 8, 3);
        }
        return maskImg;
    });

    unsigned char* srcPtr = frame.getImage().data;
    unsigned char* mskPtr = maskImg.data;
    for(int i = 0, j = 3, c = maskImg.total(); i < c; i++, j+=4) {
        srcPtr[j] = ((int)srcPtr[j] * mskPtr[i]) / 255;
    }
}
