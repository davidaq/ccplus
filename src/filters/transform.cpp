#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"

using namespace cv;
using namespace CCPlus;

CCPLUS_FILTER(transform) {
    if (parameters.size() == 0)
        return frame;
    if (parameters.size() < 12 || parameters.size() % 12 != 0) {
        log(CCPlus::logERROR) << "Not enough parameters for transform";
        return frame;
    }
    Mat finalTrans = (Mat_<double>(4, 4) << 
            frame->ext.scaleAdjustX, 0, 0, 0, 
            0, frame->ext.scaleAdjustY, 0, 0, 
            0, 0, 1, 0,
            0, 0, 0, 1);

    for (int set = 0; set < parameters.size(); set += 12) {
        float pos_x = parameters[0 + set];
        float pos_y = parameters[1 + set];
        float pos_z = parameters[2 + set];
        float anchor_x = parameters[3 + set];
        float anchor_y = parameters[4 + set];
        if(set == 0) {
            anchor_x += frame->ext.anchorAdjustX;
            anchor_y += frame->ext.anchorAdjustY;
        }
        float anchor_z = parameters[5 + set];
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

        //std::cout << "After rotate: " << std::endl << trans << std::endl;

        Mat translate_back = (Mat_<double>(4, 4) << 
                1, 0, 0, pos_x,
                0, 1, 0, pos_y, 
                0, 0, 1, pos_z,
                0, 0, 0, 1);
        trans = translate_back * trans;

        //std::cout << "After translate back: " << std::endl << trans << std::endl;

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

    Mat A = Mat::zeros(8, 8, CV_64F);
    Mat C = Mat::zeros(8, 1, CV_64F);
    int idx = 0;
    for (int i = 0; i <= 1; i++)
        for (int j = 0; j <= 1; j++) {
            int x1 = i * (frame->width - 1);
            int y1 = j * (frame->height - 1);
            Vec3f tmp = apply(finalTrans, x1, y1, 0);
            // Black magic
            double ratio = (tmp[2] + 1777) / 1777;
            float x2 = tmp[0] / ratio;
            float y2 = tmp[1] / ratio;
            //L() << x1 << " " << y1 << " " << x2 << " " << y2;

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

    /* 
     * Calculate the homography 
     **/
    invert(A, A);
    Mat H = A * C;
    H.push_back(1.0);
    H = H.reshape(0, 3);
    invert(H, H);
    float tmatrix[9];
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            tmatrix[i * 3 + j] = H.at<double>(j, i);
        }
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint trans, src_dst_size;
    GLuint program = manager->getProgram(filter_transform, &trans, &src_dst_size);
    glUseProgram(program);

    GPUFrame ret = GPUFrameCache::alloc(width, height);
    ret->bindFBO();

    glUniformMatrix3fv(trans, 1, GL_FALSE, tmatrix);

    glUniform4f(src_dst_size, frame->width, frame->height, width, height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);

    fillSprite();

    ret->ext = frame->ext;
    ret->ext.anchorAdjustX = ret->ext.anchorAdjustY = 0;
    ret->ext.scaleAdjustX = ret->ext.scaleAdjustY = 1;
    return ret;
}
