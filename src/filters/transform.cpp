#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "ccplus.hpp"

using namespace cv;
using namespace CCPlus;

Mat nextTrans(const std::vector<float> parameters, int& ptr, const GPUFrame& frame, int width, int height, bool& hadRotate) {
    if(parameters.size() - ptr < 12)
        return Mat();
    Mat finalTrans = (Mat_<double>(4, 4) << 
            frame->ext.scaleAdjustX, 0, 0, 0, 
            0, frame->ext.scaleAdjustY, 0, 0, 
            0, 0, 1, 0,
            0, 0, 0, 1);
    int sbegin = ptr;
    for (int& set = ptr; set < parameters.size(); set += 12) {
        bool bad = true;
        for(int i = 0; i < 12; i++) {
            if(abs(parameters[i + set]) > 0.0001) {
                bad = false;
                break;
            }
        }
        if(bad) {
            ptr += 12;
            break;
        }
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
        if(!hadRotate && abs(angle_x) + abs(angle_y) + abs(angle_z) > 0.000001)
            hadRotate = true;

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
    int potWidth = nearestPOT(width);
    int potHeight = nearestPOT(height);
    Mat tmp = (Mat_<double>(4, 4) << 
            potWidth * 1.0f / width, 0, 0, 0,
            0, potHeight * 1.0f / height, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
    return tmp * finalTrans;
}

CCPLUS_FILTER(transform) {
    const float & opa = parameters[0];
    int ptr = 1;
    std::vector<Mat> tlist;
    bool hadRotate = false;
    while(true) {
        const auto& m = nextTrans(parameters, ptr, frame, width, height, hadRotate);
        if(m.empty())
            break;
        tlist.push_back(m);
    }
    if (tlist.empty()) {
        log(CCPlus::logERROR) << "Not enough parameters for transform";
        return frame;
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint trans, src_dst_size, zoom, alpha;
    GLuint program = manager->getProgram(filter_transform, &trans, &src_dst_size, &zoom, &alpha);
    glUseProgram(program);

    int potWidth = nearestPOT(width);
    int potHeight = nearestPOT(height);
    GPUFrame ret = GPUFrameCache::alloc(potWidth, potHeight);

    /*****************************
     * Calculate Camera parameters
     * Currently will only support one default camera
     * TODO: Customizable camera 
     *****************************/
    float dov = 141.73 / 102.05 * width;
    glUniform1f(zoom, dov);

    glUniform4f(src_dst_size, frame->width, frame->height, potWidth, potHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureID);
    
    float tmatrix[16] = {0};
    // anti-alias
    GPUFrame antiAliasSrc;
    if(hadRotate && renderMode == FINAL_MODE) {
        tmatrix[0] = 0.9;
        tmatrix[5] = 0.9;
        tmatrix[10] = 1;
        tmatrix[15] = 1;
        glUniform1f(alpha, 1.0f);
        glUniformMatrix4fv(trans, 1, GL_FALSE, tmatrix);
        antiAliasSrc = GPUFrameCache::alloc(potWidth, potHeight);
        antiAliasSrc->bindFBO();
        fillSprite();
        glBindTexture(GL_TEXTURE_2D, antiAliasSrc->textureID);
    }
    ret->bindFBO();

    glUniform1f(alpha, opa / tlist.size());
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for(Mat t : tlist) {
        if(hadRotate && renderMode == FINAL_MODE) {
            Mat tmp = (Mat_<double>(4, 4) << 
                    1/0.9, 0, 0, 0,
                    0, 1/0.9, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
            t =  t * tmp;
        }
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                tmatrix[i * 4 + j] = t.at<double>(j, i);
            }
        }
        glUniformMatrix4fv(trans, 1, GL_FALSE, tmatrix);
        fillSprite();
    }
    glDisable(GL_BLEND);

    ret->ext = frame->ext;
    ret->ext.anchorAdjustX = ret->ext.anchorAdjustY = 0;
    ret->ext.scaleAdjustX = ret->ext.scaleAdjustY = 1;
    return ret;
}
