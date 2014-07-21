#include "filter.hpp"

using namespace cv;

CCPLUS_FILTER(transform) {
    // TODO: find a better way to map parameter   
    int pos_row = (int)parameters[0];
    int pos_col = (int)parameters[1];
    //int anchor_row = (int)parameters[2];
    //int anchor_col = (int)parameters[3];
    //float scale_x = parameters[4];
    //float scale_y = parameters[5];
    //float rotate = parameters[6];

    Mat tmp(input, Rect(pos_col, pos_row, input.cols, input.rows));
    return tmp;
}
