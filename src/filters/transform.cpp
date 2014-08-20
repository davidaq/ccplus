#include "filter.hpp"
#include "logger.hpp"
#include "mat-cache.hpp"
#include <cmath>

#ifndef __ANDROID__
#define round(X) std::round(X)
#endif

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(transform) {
    if (parameters.size() == 0)
        return;
    if (parameters.size() < 12 || parameters.size() % 12 != 0) {
        log(CCPlus::logERROR) << "Not enough parameters for transform";
        return;
    }
    Mat& input = frame.getImage();
    Mat finalTrans = Mat::eye(4, 4, CV_64F);
    for (int set = 0; set < parameters.size(); set += 12) {
        int pos_x = (int)parameters[0 + set];
        int pos_y = (int)parameters[1 + set];
        int pos_z = (int)parameters[2 + set];
        int anchor_x = (int)parameters[3 + set];
        int anchor_y = (int)parameters[4 + set];
        int anchor_z = (int)parameters[5 + set];
        if (anchor_z != 0) {
            log(CCPlus::logWARN) << "Corrently anchor z is not supported";
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

    // Boundary of src image
    int top_bound = 0;
    int left_bound = 0;
    int right_bound = input.cols;
    int down_bound = input.rows;

    auto bilinear_interpolate = [] (Mat mat, float x, float y) {
        float x1 = std::floor(x);
        float y1 = std::floor(y);
        float x2 = std::ceil(x);
        float y2 = std::ceil(y);
        
        auto pixel = [&mat] (int y, int x) {
            Vec4b ret;
            if(y >= mat.rows || y < 0 || x >= mat.cols || x < 0)
                ret = Vec4b(0, 0, 0, 0);
            else
                ret = mat.at<Vec4b>(y, x);
            return ret;
        };
        if (x1 == x2 && y1 == y2) 
            return pixel(round(y1), round(x1));
        else if (x1 == x2) {
            // y direction interpolation
            Vec4b r1 = pixel(y1, x1);
            Vec4b r2 = pixel(y2, x1);
            Vec4b ans; 
            for (int i = 0; i < 4; i++) {
                ans[i] = round(r1[i] * (y2 - y) / (y2 - y1) + r2[i] * (y - y1) / (y2 - y1));
            }
            return ans;
        } else if (y1 == y2) {
            Vec4b r1 = pixel(y1, x1);
            Vec4b r2 = pixel(y1, x2);
            Vec4b ans; 
            for (int i = 0; i < 4; i++) {
                ans[i] = round(r1[i] * (x2 - x) / (x2 - x1) + r2[i] * (x - x1) / (x2 - x1));
            }
            return ans;
        }
        
        Vec4b p11 = pixel(y1, x1);
        Vec4b p21 = pixel(y1, x2);
        Vec4b p12 = pixel(y2, x1);
        Vec4b p22 = pixel(y2, x2);

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
    Mat ret(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    yMin = max(yMin, .0f);
    yMax = min(yMax + 1, (float)height);
    xMin = max(xMin, .0f);
    xMax = min(xMax + 1, (float)width);
    for (int i = yMin; i < yMax; i++)
        for (int j = xMin; j < xMax; j++) {
            float x = H.at<double>(0, 0) * j + H.at<double>(0, 1) * i + H.at<double>(0, 2);
            float y = H.at<double>(1, 0) * j + H.at<double>(1, 1) * i + H.at<double>(1, 2);
            float z = H.at<double>(2, 0) * j + H.at<double>(2, 1) * i + H.at<double>(2, 2);
            x /= z;
            y /= z;

            // Nomalize
            if (y < top_bound  || y >= down_bound || 
                x < left_bound || x >= right_bound)
                continue;
            ret.at<Vec4b>(i, j) = bilinear_interpolate(input, x, y);
        }


    // Update frame
    frame.setImage(ret);
}
