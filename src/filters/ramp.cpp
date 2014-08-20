#include "filter.hpp"
#include "logger.hpp"
#include "frame.hpp"
#include "mat-cache.hpp"

#include <cmath>

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(ramp) {
    if (parameters.size() == 0)
        return;
    if (parameters.size() < 12) {
        log(CCPlus::logERROR) << "Insufficient parameters for ramp effect";
        return;
    }
    
    width = frame.getWidth();
    height = frame.getHeight();
    
    float alpha = parameters[11];
    if (alpha < 0)
        alpha = 0;
    if(alpha > 1)
        alpha = 1;
    HashFactory hash;
    hash << "RAMP" << width << height;
    for(int i = 0; i < 10; i++) {
        hash << parameters[i];
    }
    hash << alpha;
    cv::Mat ret = MatCache::get(hash.str(), [width, height, alpha, &parameters]() {
        
        int type = parameters[0];
        int start_x = parameters[1];
        int start_y = parameters[2];
        int sr = parameters[3];
        int sg = parameters[4];
        int sb = parameters[5];
        
        int end_x = parameters[6];
        int end_y = parameters[7];
        int er = parameters[8];
        int eg = parameters[9];
        int eb = parameters[10];
        
        Mat ret = Mat::zeros(height, width, CV_8UC4);
        
        float dx = start_x - end_x;
        float dy = start_y - end_y;
        // TODO: implemented a fast sqrt or get rid of it
        float dis = sqrt(dx * dx + dy * dy);
        
        auto getIntensity =
        [type, start_x, start_y, end_x, end_y, dis] (int x, int y) {
            if (type < 0) {
                // Linear ramp
                float vx = x - start_x;
                float vy = y - start_y;
                
                float dx = end_x - start_x;
                float dy = end_y - start_y;
                
                float prj_len = vx * dx + vy * dy;
                prj_len /= dis;
                return prj_len / dis;
            } else {
                // Radial ramp
                float dx = x - start_x;
                float dy = y - start_y;
                return sqrt(dx * dx + dy * dy) / dis;
            }
        };
#define uchar unsigned char
        for (int i = 0; i < width; i++)
            for (int j = 0; j < height; j++) {
                float intensity = getIntensity(i, j);
                float diff_r = er - sr;
                float diff_g = eg - sg;
                float diff_b = eb - sb;
                ret.at<Vec4b>(j, i)[0] = (uchar) std::min(255.0f,
                                                          std::max(0.0f, diff_r * intensity + sr));
                ret.at<Vec4b>(j, i)[1] = (uchar) std::min(255.0f,
                                                          std::max(0.0f, diff_g * intensity + sg));
                ret.at<Vec4b>(j, i)[2] = (uchar) std::min(255.0f,
                                                          std::max(0.0f, diff_b * intensity + sb));
                
                ret.at<Vec4b>(j, i)[3] = 255;
            }
        return ret;
    });
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++) {
            frame.getImage().at<Vec4b>(j, i)[3] = (unsigned char) 255 * alpha;
        }
    frame.mergeFrame(Frame(ret));
}
