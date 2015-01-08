#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"

using namespace CCPlus;
using namespace cv;

CCPLUS_FILTER(4corner) {
    if (parameters.size() < 8) {
        log(CCPlus::logERROR) << "Not enough parameters for 4 corner pin";
        return frame;
    }
    /***********************
     * Homography
     ********************/
    Mat A(8, 8, CV_64F);
    Mat C(8, 1, CV_64F);
    int idx = 0;
    double maxX, minX, maxY, minY;
    maxX = frame->width;
    maxY = frame->height;
    minX = minY = 0;
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            double x1 = frame->width * j;
            double y1 = frame->height * i;
            double x2 = parameters[idx * 2] / frame->ext.scaleAdjustX;
            double y2 = parameters[idx * 2 + 1] / frame->ext.scaleAdjustY;

            maxX = std::max(maxX, x2);
            minX = std::min(minX, x2);
            maxY = std::max(maxY, y2);
            minY = std::min(minY, y2);

            A.at<double>(idx * 2, 0) = x1; 
            A.at<double>(idx * 2, 1) = y1;
            A.at<double>(idx * 2, 2) = 1;
            A.at<double>(idx * 2, 3) = 0;
            A.at<double>(idx * 2, 4) = 0;
            A.at<double>(idx * 2, 5) = 0;
            A.at<double>(idx * 2, 6) = -x2 * x1;
            A.at<double>(idx * 2, 7) = -x2 * y1;
            C.at<double>(idx * 2, 0) = x2;
            
            A.at<double>(idx * 2 + 1, 0) = 0; 
            A.at<double>(idx * 2 + 1, 1) = 0;
            A.at<double>(idx * 2 + 1, 2) = 0;
            A.at<double>(idx * 2 + 1, 3) = x1;
            A.at<double>(idx * 2 + 1, 4) = y1;
            A.at<double>(idx * 2 + 1, 5) = 1;
            A.at<double>(idx * 2 + 1, 6) = -y2 * x1;
            A.at<double>(idx * 2 + 1, 7) = -y2 * y1;
            C.at<double>(idx * 2 + 1, 0) = y2;

            idx++;
        }
    }
    invert(A, A);
    Mat H = A * C;
    H.push_back(1.0);
    H = H.reshape(0, 3);

    float homography[9];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            homography[i * 3 + j] = H.at<double>(j, i);
        }
    }

    int retWidth = nearestPOT(maxX - minX + 1, true);
    int retHeight = nearestPOT(maxY - minY + 1, true);

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint trans, src_dst_size, transition;
    GLuint program = manager->getProgram(filter_4corner, &trans, &src_dst_size, &transition);
    glUseProgram(program);

    GPUFrame ret = GPUFrameCache::alloc(retWidth, retHeight);
    ret->bindFBO();
    ret->ext = frame->ext;
    float transition_x = std::max(0.0, -minX);
    float transition_y = std::max(0.0, -minY);
    ret->ext.anchorAdjustX += transition_x;
    ret->ext.anchorAdjustY += transition_y;

    glUniformMatrix3fv(trans, 1, GL_FALSE, homography);
    glUniform4f(src_dst_size, frame->width, frame->height, retWidth, retHeight);
    glUniform2f(transition, transition_x, transition_y);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    //fillSprite();
    fillDensedSprite();

    return ret;
}
