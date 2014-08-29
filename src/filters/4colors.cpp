#include "filter.hpp"
#include "logger.hpp"

#include "frame.hpp"

#include <cmath>
#include <vector>

using namespace std;
using namespace CCPlus;
using namespace cv;

/*
 * Thanks Lagrange
 */
CCPLUS_FILTER(4color) {
    if (parameters.size() == 0) 
        return;
    if (parameters.size() < 23) {
        log(CCPlus::logERROR) << "Insufficient parameters for 4 color effect";
        return;
    }
    // Initializing
    std::vector<float> rgbs; 
    std::vector<float> xs, ys; 
    
    for (int i = 0; i < 20; i+=5) {
        xs.push_back(parameters[i + 0]);
        ys.push_back(parameters[i + 1]);
        rgbs.push_back(parameters[i + 2]);
        rgbs.push_back(parameters[i + 3]);
        rgbs.push_back(parameters[i + 4]);
    }
    float blend = parameters[20];
    float opacity = parameters[21];
    int mode = parameters[22];

    Mat mask = Mat(frame.getHeight(), frame.getWidth(), CV_8UC4, {0, 0, 0, opacity * 255});

    // TODO: optimizting on math. 
    // Try to calculate multiple channels together
    for (int ch = 0; ch < 3; ch++) {
        // A * x = C
        // 4 x 6 * 6 x 1 = 4 x 1
        Mat A = Mat::zeros(4, 6, CV_64F);
        Mat C = Mat::zeros(4, 1, CV_64F);
        for (int i = 0; i < 4; i++) {
            C.at<double>(i, 0) = rgbs[i * 3 + ch];
        }

        for (int i = 0; i < 4; i++) {
            A.at<double>(i, 0) = xs[i] * xs[i];
            A.at<double>(i, 1) = xs[i] * ys[i];
            A.at<double>(i, 2) = ys[i] * ys[i];
            A.at<double>(i, 3) = xs[i];
            A.at<double>(i, 4) = ys[i];
            A.at<double>(i, 5) = 1;
        }
        Mat H = Mat::zeros(10, 10, CV_64F);
        for (int i = 0; i < 3; i++)
            H.at<double>(i, i) = 1;
        A.copyTo(H(Rect_<int>(0, 6, 6, 4)));
        Mat tmp = A.t();
        tmp.copyTo(H(Rect_<int>(6, 0, 4, 6)));

        // A * [a; lambda] = B
        Mat B = Mat::zeros(10, 1, CV_64F);
        for (int i = 0; i < 4; i++)
            B.at<double>(i + 6, 0) = C.at<double>(i, 0);

        if (std::abs(determinant(H)) < 0.001) {
            log(CCPlus::logWARN) << "Can't generate bilinear from this 4 points.";
        }

        invert(H, H);
        Mat ret = H * B;

        auto calcColor = [&ret](int x, int y) {
            auto p = [&ret](int i) {
                return ret.at<double>(i, 0);
            };
            float tmp = p(0) * x * x + p(1) * x * y + 
                p(2) * y * y + p(3) * x +
                p(4) * y + p(5);
            tmp = std::max<float>(0.0f, tmp);
            tmp = std::min<float>(255.0f, tmp);
            return tmp;
        };

        cv::Vec4b* pixelMask = mask.ptr<Vec4b>(0);
        int rows = frame.getHeight();
        int cols = frame.getWidth();
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                pixelMask[i * cols + j][ch] = calcColor(j, i);
            }
        }
    }
    int tmp = blend;
    tmp = tmp % 2 == 0 ? tmp + 1 : tmp;
    GaussianBlur(mask, mask, {tmp, tmp}, 0, 0);
    Frame maskFrame(mask);
    maskFrame.mergeFrame(frame, mode);
    frame = maskFrame;
}
