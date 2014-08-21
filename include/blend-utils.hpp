#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

enum BlendMode {
    NONE = -1, DEFAUT=0, ADD, MULTIPLY, SCREEN
};

#define BLENDER std::function<cv::Vec4b(cv::Vec4b, cv::Vec4b)>

const BLENDER& getBlender(int mode);

Vec4b defaultBlend(Vec4b top, Vec4b down);
Vec4b noneBlend(Vec4b top, Vec4b down);
Vec4b addBlend(Vec4b top, Vec4b down);
Vec4b multiplyBlend(Vec4b top, Vec4b down);
Vec4b screenBlend(Vec4b top, Vec4b down);
