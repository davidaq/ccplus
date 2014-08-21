#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

enum BlendMode {
    DEFAUT=-1, NONE, ADD, MULTIPLY, 
};

#define BLENDER std::function<cv::Vec4b(cv::Vec4b, cv::Vec4b)>

const BLENDER& getBlender(int mode);

Vec4b defaultBlend(Vec4b top, Vec4b down);
