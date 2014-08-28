#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

/*
 * Follow the SVG specification
 * http://www.w3.org/TR/SVG/filters.html#feBlendElement
 *
 * Some of them are using formulas in 
 * http://www.pegtop.net/delphi/articles/blendmodes/lighten.htm
 */

enum BlendMode {
    NONE = -1, DEFAUT = 0,
    ADD, MULTIPLY, SCREEN, DISOLVE, DARKEN = 5,
    LIGHTEN, OVERLAY, DIFFERENCE 
};

/*
 * Parameters
 * @ca premultiplied fg color 0 ~ 1
 * @cb premultiplied bg color 0 ~ 1
 * @qa fg opacity 0 ~ 1
 * @qa bg opacity 0 ~ 1
 *
 * Return
 * @ Premultiplied color 0 ~ 255
 */
#define BLENDER_CORE std::function<float(float, float, float, float)>
#define BLENDER std::function<cv::Vec4b(cv::Vec4b, cv::Vec4b)>

BLENDER getBlender(int mode);

float defaultBlend(float ca, float cb, float qa, float qb);
float noneBlend(float ca, float cb, float qa, float qb);
float addBlend(float ca, float cb, float qa, float qb);
float multiplyBlend(float ca, float cb, float qa, float qb);
float screenBlend(float ca, float cb, float qa, float qb);
float disolveBlend(float ca, float cb, float qa, float qb);
float darkenBlend(float ca, float cb, float qa, float qb);
float lightenBlend(float ca, float cb, float qa, float qb);
float overlayBlend(float ca, float cb, float qa, float qb);
float differenceBlend(float ca, float cb, float qa, float qb);
