#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(4color) {
    if (parameters.size() < 23) {
        log(CCPlus::logERROR) << "Insufficient parameters for 4 color effect";
        return GPUFrame();
    }
    // Initializing
    std::vector<float> rgbs; 
    std::vector<float> xs, ys; 
    
    for (int i = 0; i < 20; i+=5) {
        xs.push_back(parameters[i + 0] / frame->width);
        ys.push_back(parameters[i + 1] / frame->height);
        rgbs.push_back(parameters[i + 2] / 255.0f);
        rgbs.push_back(parameters[i + 3] / 255.0f);
        rgbs.push_back(parameters[i + 4] / 255.0f);
    }
    float blend = parameters[20];
    float opacity = parameters[21];
    int mode = parameters[22];

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_4color",
            "shaders/fill.v.glsl",
            "shaders/filters/4color.f.glsl"
            );
    glUseProgram(program);

    glUniform1f(glGetUniformLocation(program, "opacity"), opacity);

    GPUFrame tmp_frame = GPUFrameCache::alloc(frame->width, frame->height);
    tmp_frame->bindFBO();

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

        if (std::abs(determinant(H)) < 0.00001) {
            log(CCPlus::logWARN) << "Can't generate bilinear from this 4 points.";
        }

        invert(H, H);
        Mat ret = H * B;

        float prms[6];
        for (int i = 0; i < 6; i++)
            prms[i] = ret.at<double>(i, 0);

        switch (ch) {
            case 0:
                glUniform1fv(glGetUniformLocation(program, "params_b"), 6, prms);
                break;
            case 1:
                glUniform1fv(glGetUniformLocation(program, "params_g"), 6, prms);
                break;
            case 2:
                glUniform1fv(glGetUniformLocation(program, "params_r"), 6, prms);
                break;
            default:
                break;
        }
    }
    fillSprite();

    return mergeFrame(frame, tmp_frame, (BlendMode)mode);
}
