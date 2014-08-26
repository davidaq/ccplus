#include "filter.hpp"
#include "logger.hpp"
#include "mat-cache.hpp"
#include <cmath>

#ifndef __ANDROID__
#define round(X) std::round(X)
#endif

using namespace cv;
using namespace CCPlus;

/*
 * Another atempt to implement transform filter using more opencv
 */

CCPLUS_FILTER(transform) {
    if (parameters.size() == 0)
        return;
    if (parameters.size() < 12 || parameters.size() % 12 != 0) {
        log(CCPlus::logERROR) << "Not enough parameters for transform";
        return;
    }
    Mat& input = frame.getImage();
    if (input.empty()) {
        return;
    }
    Mat finalTrans = Mat::eye(4, 4, CV_64F);
    for (int set = 0; set < parameters.size(); set += 12) {
        int pos_x = (int)parameters[0 + set];
        int pos_y = (int)parameters[1 + set];
        int pos_z = (int)parameters[2 + set];
        int anchor_x = (int)parameters[3 + set];
        int anchor_y = (int)parameters[4 + set];
        int anchor_z = (int)parameters[5 + set];
        if (anchor_z != 0) {
            log(CCPlus::logWARN) << "Anchor z is not supported";
        }
        float scale_x = parameters[6 + set];
        float scale_y = parameters[7 + set];
        float scale_z = parameters[8 + set];
        float angle_x = parameters[9 + set];
        float angle_y = parameters[10 + set];
        float angle_z = parameters[11 + set];

        // Put original image into the large layer image 
        angle_x = angle_x * M_PI / 180.0;
        double cx = cos(angle_x);
        double sx = sin(angle_x);
        angle_y = angle_y * M_PI / 180.0;
        double cy = cos(angle_y);
        double sy = sin(angle_y);
        angle_z = angle_z * M_PI / 180.0;
        double cz = cos(angle_z);
        double sz = sin(angle_z);

        Mat trans = (Mat_<double>(4, 4) << 
                1, 0, 0, -anchor_x, 
                0, 1, 0, -anchor_y, 
                0, 0, 1, -anchor_z,
                0, 0, 0, 1);

        //std::cout << "====================================" << std::endl;
        //std::cout << "Init : " << std::endl << trans << std::endl;

        Mat scale = (Mat_<double>(4, 4) << 
                scale_x, 0, 0, 0,
                0, scale_y, 0, 0,
                0, 0, scale_z, 0,
                0, 0, 0, 1);
        trans = scale * trans;

        //std::cout << "After scale: " << std::endl << trans << std::endl;

        Mat rotate = (Mat_<double>(4, 4) << 
                cy * cz,                -cy * sz,               sy,         0,
                cz * sx * sy + cx * sz, cx * cz - sx * sy * sz, -cy * sx,   0,
                -cx * cz * sy + sx * sz, cz * sx + sx * sy * sz, cx * cy,   0,
                0,                      0,                      0,          1);
        Mat tmp = (Mat_<double>(4, 4) << 
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 1,
                0, 0, 0, 1);
        trans = rotate * trans;

        //std::cout << "Rotation mat" << std::endl << rotate << std::endl;
        //std::cout << "After rotate: " << std::endl << trans << std::endl;

        Mat translate_back = (Mat_<double>(4, 4) << 
                1, 0, 0, pos_x,
                0, 1, 0, pos_y, 
                0, 0, 1, pos_z,
                0, 0, 0, 1);
        trans = translate_back * trans;

        //std::cout << "After translate back: " << std::endl << trans << std::endl;

        //invert(trans, trans);
        finalTrans = trans * finalTrans;
    }

    auto at = [&finalTrans] (int i, int j) {
        return finalTrans.at<double>(i, j);
    };
    auto nonZero = [&at] (int i, int j) {
        return std::abs(at(i, j)) > 0.00001;
    };
    bool affine = true;
    if (nonZero(0, 2) || nonZero(1, 2) || nonZero(2, 0) ||
        nonZero(2, 1) || nonZero(2, 3)) 
        affine = false;
    if (affine) {
        Mat affineMat = (Mat_<double>(2, 3) << 
                at(0, 0), at(0, 1), at(0, 3),
                at(1, 0), at(1, 1), at(1, 3));
        Mat ret(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        profile(Filter_transform_affine) {
            warpAffine(input, ret, affineMat, {width, height});
        }
        frame.setImage(ret);
        return;
    }

    auto apply = [](Mat trans, float x, float y, float z) {
        double noer = trans.at<double>(3, 0) * x + trans.at<double>(3, 1) * y + 
            trans.at<double>(3, 2) * z + trans.at<double>(3, 3);

        double nx = trans.at<double>(0, 0) * x + trans.at<double>(0, 1) * y + 
            trans.at<double>(0, 2) * z + trans.at<double>(0, 3);
        nx /= noer;
        double ny = trans.at<double>(1, 0) * x + trans.at<double>(1, 1) * y + 
            trans.at<double>(1, 2) * z + trans.at<double>(1, 3);
        ny /= noer;
        double nz = trans.at<double>(2, 0) * x + trans.at<double>(2, 1) * y + 
            trans.at<double>(2, 2) * z + trans.at<double>(2, 3);
        nz /= noer;
        return Vec3f(nx, ny, nz);
    };

    float xMax = 0, xMin = width - 1, yMax = 0, yMin = height - 1;

    Mat A = Mat::zeros(8, 8, CV_64F);
    Mat C = Mat::zeros(8, 1, CV_64F);
    int idx = 0;
    for (int i = 0; i <= 1; i++)
        for (int j = 0; j <= 1; j++) {
            int x1 = i * (frame.getImage().cols - 1);
            int y1 = j * (frame.getImage().rows - 1);
            Vec3f tmp = apply(finalTrans, x1, y1, 0);
            // Black magic
            double ratio = (tmp[2] + 1777) / 1777;
            float x2 = tmp[0] / ratio;
            float y2 = tmp[1] / ratio;
            //L() << x1 << " " << y1 << " " << x2 << " " << y2;
            xMax = max(xMax, x2);
            xMin = min(xMin, x2);
            yMax = max(yMax, y2);
            yMin = min(yMin, y2);

            A.at<double>(idx * 2, 0) = -x2;
            A.at<double>(idx * 2, 1) = -y2;
            A.at<double>(idx * 2, 2) = -1;
            A.at<double>(idx * 2, 6) = (x1 * x2);
            A.at<double>(idx * 2, 7) = (x1 * y2);
            C.at<double>(idx * 2, 0) = -x1;

            A.at<double>(idx * 2 + 1, 3) = -x2;
            A.at<double>(idx * 2 + 1, 4) = -y2;
            A.at<double>(idx * 2 + 1, 5) = -1;
            A.at<double>(idx * 2 + 1, 6) = (y1 * x2);
            A.at<double>(idx * 2 + 1, 7) = (y1 * y2);
            C.at<double>(idx * 2 + 1, 0) = -y1;
            idx++;
        }

    invert(A, A);
    Mat H = A * C;
    H.push_back(1.0);
    H = H.reshape(0, 3);

    Mat ret(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::Mat pixelMappingX(ret.size(), CV_32FC1, cv::Scalar(-1.f));
    cv::Mat pixelMappingY(ret.size(), CV_32FC1, cv::Scalar(-1.f));

    float tmatrix[3][3];
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++) {
            tmatrix[i][j] = H.at<double>(i, j);
        }
    xMin = std::max(xMin, 0.f);
    xMax = std::min(xMax, (float)width);
    yMin = std::max(yMin, 0.f);
    yMax = std::min(yMax, (float)height);
    profile(Filter_transform_map_func) {
        for (int i = yMin; i < yMax; i++)
            for (int j = xMin; j < xMax; j++) {
                float x = tmatrix[0][0] * j + tmatrix[0][1] * i + tmatrix[0][2];
                float y = tmatrix[1][0] * j + tmatrix[1][1] * i + tmatrix[1][2];
                float z = tmatrix[2][0] * j + tmatrix[2][1] * i + tmatrix[2][2];

                x /= z;
                y /= z;
                pixelMappingX.at<float>(i, j) = x;
                pixelMappingY.at<float>(i, j) = y;
            }
    }
    profile(Filter_transform_remap) {
        cv::remap(input, ret, pixelMappingX, pixelMappingY, CV_INTER_LINEAR, BORDER_TRANSPARENT);
    }
    frame.setImage(ret);
}
