#include "filter.hpp"
#include <cmath>

using namespace cv;

CCPLUS_FILTER(transform) {
    // TODO: find a better way to map parameter   
    int pos_row = (int)parameters[0];
    int pos_col = (int)parameters[1];
    int anchor_row = (int)parameters[2];
    int anchor_col = (int)parameters[3];
    float scale_row = parameters[4];
    float scale_col = parameters[5];
    float angle = parameters[6];

    // Put original image into the large layer image 
    Mat ret(height, width, CV_8UC4);
    angle = angle * M_PI / 180.0;
    double ct = cos(angle);
    double st = sin(angle);

    Mat trans = (Mat_<double>(3, 3) << 
            1, 0, pos_col + anchor_col, 
            0, 1, pos_row + anchor_row, 
            0, 0, 1);

    //std::cout << "Init : " << std::endl << trans << std::endl;

    Mat scale = (Mat_<double>(3, 3) << 
            scale_col, 0, 0,
            0, scale_row, 0,
            0, 0, 1);
    trans = scale * trans;

    //std::cout << "After scale: " << std::endl << trans << std::endl;

    Mat rotate = (Mat_<double>(3, 3) << 
            ct, st, 0,
            -st, ct, 0, 
            0, 0, 1);
    trans = rotate * trans;

    std::cout << "After rotate: " << std::endl << trans << std::endl;

    Mat translate_back = (Mat_<double>(3, 3) << 
            1, 0, -pos_col - anchor_col,
            0, 1, -pos_row - anchor_row, 
            0, 0, 1);
    trans = translate_back * trans;

    //std::cout << "After translate back: " << std::endl << trans << std::endl;

    if (std::abs(determinant(trans) - 0.0) < 0.001) {
        // Not invertable
        throw std::invalid_argument("Arguments results in an uninvertable matrix");
    }
    invert(trans, trans);

    auto bilinear_interpolate = []() {
        // TODO TODO TODO TODO TODO
        // TLTDI = TOO LAZY TO DO IT
    };
    
    // Boundary of src image
    int top_bound = pos_row - anchor_row;
    int left_bound = pos_col - anchor_col;
    int right_bound = left_bound + input.cols;
    int down_bound = top_bound + input.rows;

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            Mat pos = trans * (Mat_<double>(3, 1) << j , i, 1);

            // Nomalize
            double x = pos.at<double>(0, 0) / pos.at<double>(2, 0);
            double y = pos.at<double>(1, 0) / pos.at<double>(2, 0);
            if (y < (double)top_bound  || y >= (double)down_bound || 
                x < (double)left_bound || x >= (double)right_bound)
                continue;
            // TODO: bilinear interpolate   
            int ix = std::round(x);
            int iy = std::round(y);
            ret.at<Vec4b>(i, j) = input.at<Vec4b>(iy - top_bound, ix - left_bound);
        }
    return ret;
}
