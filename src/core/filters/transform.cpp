#include "filter.hpp"

using namespace cv;

CCPLUS_FILTER(transform) {
    // TODO: find a better way to map parameter   
    int pos_row = (int)parameters[0];
    int pos_col = (int)parameters[1];
    int anchor_row = (int)parameters[2];
    int anchor_col = (int)parameters[3];
    //float scale_x = parameters[4];
    //float scale_y = parameters[5];
    //float rotate = parameters[6];

    // Put original image into the large layer image 
    Mat ret(height, width, CV_8UC4);
    int top_bound = pos_row - anchor_row;
    int left_bound = pos_col - anchor_col;
    int right_bound = left_bound + input.cols;
    int down_bound = top_bound + input.rows;
    // Make sure they intersect
    if (right_bound > 0 && down_bound > 0 && 
        left_bound < width && top_bound < height) {
        int real_top = std::max(top_bound, 0);
        int real_left = std::max(left_bound, 0);
        int real_right = std::min(right_bound, width);
        int real_down = std::min(down_bound, height);

        Mat tmp(ret, Rect(real_left, real_top, real_right - real_left, real_down - real_top));

        int inner_top = real_top - top_bound;
        int inner_left = real_left - left_bound;
        int inner_right  = inner_left + (real_right - real_left);
        int inner_down  = inner_top + (real_down - real_top);
        input(Range(inner_top, inner_down), Range(inner_left, inner_right)).copyTo(tmp);
    }
    return ret;
}
