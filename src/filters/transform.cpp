#include "filter.hpp"
#include <cmath>

#ifndef __ANDROID__
#define round(X) std::round(X)
#endif

using namespace cv;

CCPLUS_FILTER(transform) {
    if (parameters.size() == 0)
        return;
    int pos_row = (int)parameters[0];
    int pos_col = (int)parameters[1];
    int anchor_row = (int)parameters[2];
    int anchor_col = (int)parameters[3];
    float scale_row = parameters[4];
    float scale_col = parameters[5];
    float angle = parameters[6];

    Mat& input = frame.getImage();

    // Put original image into the large layer image 
    Mat ret(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    angle = angle * M_PI / 180.0;
    double ct = cos(angle);
    double st = sin(angle);

    Mat trans = (Mat_<double>(3, 3) << 
            1, 0, -pos_col, 
            0, 1, -pos_row, 
            0, 0, 1);

    //std::cout << "====================================" << std::endl;
    //std::cout << "Init : " << std::endl << trans << std::endl;

    Mat scale = (Mat_<double>(3, 3) << 
            scale_col, 0, 0,
            0, scale_row, 0,
            0, 0, 1);
    trans = scale * trans;

    //std::cout << "After scale: " << std::endl << trans << std::endl;

    Mat rotate = (Mat_<double>(3, 3) << 
            ct, -st, 0,
            st, ct, 0, 
            0, 0, 1);
    trans = rotate * trans;

    //std::cout << "After rotate: " << std::endl << trans << std::endl;

    Mat translate_back = (Mat_<double>(3, 3) << 
            1, 0, pos_col,
            0, 1, pos_row, 
            0, 0, 1);
    trans = translate_back * trans;

    //std::cout << "After translate back: " << std::endl << trans << std::endl;

    if (std::abs(determinant(trans) - 0.0) < 0.001) {
        // Not invertable
        throw std::invalid_argument("Arguments results in an uninvertable matrix");
    }
    invert(trans, trans);

    auto bilinear_interpolate = [] (Mat mat, float x, float y) {
        float x1 = std::floor(x);
        float y1 = std::floor(y);
        float x2 = std::ceil(x);
        float y2 = std::ceil(y);
        float xr = round(x);
        float yr = round(y);
        if(x2 >= mat.cols)
            x2 = mat.cols - 1;
        if(y2 >= mat.rows)
            y2 = mat.rows - 1;
        if(xr >= mat.cols)
            xr = mat.cols - 1;
        if(yr >= mat.rows)
            yr = mat.rows - 1;
        if (x1 == x2 && y1 == y2) 
            return mat.at<Vec4b>(yr, xr);
        else if (x1 == x2) {
            // y direction interpolation
            Vec4b r1 = mat.at<Vec4b>(y1, x1);
            Vec4b r2 = mat.at<Vec4b>(y2, x1);
            Vec4b ans; 
            for (int i = 0; i < 4; i++) {
                ans[i] = round(r1[i] * (y2 - y) / (y2 - y1) + r2[i] * (y - y1) / (y2 - y1));
            }
            return ans;
        } else if (y1 == y2) {
            Vec4b r1 = mat.at<Vec4b>(y1, x1);
            Vec4b r2 = mat.at<Vec4b>(y1, x2);
            Vec4b ans; 
            for (int i = 0; i < 4; i++) {
                ans[i] = round(r1[i] * (x2 - x) / (x2 - x1) + r2[i] * (x - x1) / (x2 - x1));
            }
            return ans;
        }
        
        Vec4b p11 = mat.at<Vec4b>(y1, x1);
        Vec4b p21 = mat.at<Vec4b>(y1, x2);
        Vec4b p12 = mat.at<Vec4b>(y2, x1);
        Vec4b p22 = mat.at<Vec4b>(y2, x2);

        float tmp1 = 1 / ((x2 - x1) * (y2 - y1));
        Vec4b ans(0, 0, 0, 0); 
        for (int i = 0; i < 4; i++) {
            float t = tmp1 * 
                (p11[i] * (x2 - x) * (y2 - y) + 
                 p21[i] * (x - x1) * (y2 - y) + 
                 p12[i] * (x2 - x) * (y - y1) +
                 p22[i] * (x - x1) * (y - y1));
            ans[i] = round(t);
        } 
        return ans;
    };
    
    // Boundary of src image
    int top_bound = pos_row - anchor_row;
    int left_bound = pos_col - anchor_col;
    int right_bound = left_bound + input.cols;
    int down_bound = top_bound + input.rows;

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            double z = trans.at<double>(2, 0) * j + trans.at<double>(2, 1) * i + trans.at<double>(2, 2);
            double x = trans.at<double>(0, 0) * j + trans.at<double>(0, 1) * i + trans.at<double>(0, 2) / z;
            double y = trans.at<double>(1, 0) * j + trans.at<double>(1, 1) * i + trans.at<double>(1, 2) / z;

            // Nomalize
            if (y < (double)top_bound  || y >= (double)down_bound || 
                x < (double)left_bound || x >= (double)right_bound)
                continue;
            ret.at<Vec4b>(i, j) = bilinear_interpolate(input, x - left_bound, y - top_bound);
        }


    // Update frame
    frame.setImage(ret);
}
