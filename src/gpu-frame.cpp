#include "gpu-frame.hpp"
#include "frame.hpp"

using namespace CCPlus;

void GPUFrame::destroy() {
    if(textureID) {
        glDeleteTextures(&textureID, 1);
        audio = cv::Mat();
    }
}

Frame GPUFrame::toCPU() {
    Frame ret;
    ret.audio = audio.clone();
    if(textureID) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        ret.image = cv::Mat(height, width, CV_8UC4);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, ret.image.data);
    }
    return ret;
}

void GPUFrame::load(const Frame& frame) {
    width = frame.image.cols;
    height = frame.image.rows;
    audio = frame.audio.clone();
    if(width * height > 0) {
        if(!textureID) {
            glGenTextures(&textureID, 1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA8, GL_UNSIGNED_BYTE, frame.image.data);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glSubTexImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA8, GL_UNSIGNED_BYTE, frame.image.data);
        }
    }
}

