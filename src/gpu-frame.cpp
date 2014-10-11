#include "gpu-frame.hpp"
#include "frame.hpp"

using namespace CCPlus;

void GPUFrame::destroy() {
    if(fboID) {
        glDeleteFramebuffers(1, &fboID);
        fboID = 0;
    }
    if(textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    audio = cv::Mat();
}

void GPUFrame::bindFBO() {
    if(fboID) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glViewport(0, 0, width, height);
    } else if(textureID) {
        glGenFramebuffers(1, &fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        glViewport(0, 0, width, height);
    }
}

void GPUFrame::createTexture(int width, int height, void* data) {
    if(!textureID) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);

        this->width = width;
        this->height = height;
    }
}

Frame GPUFrame::toCPU() {
    Frame ret;
    ret.audio = audio.clone();
    if(textureID) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        ret.image = cv::Mat::zeros(height, width, CV_8UC4);
        bindFBO();
        glReadPixels(0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, ret.image.data);
    }
    return ret;
}

void GPUFrame::load(const Frame& frame) {
    width = frame.image.cols;
    height = frame.image.rows;
    audio = frame.audio.clone();
    if(width * height > 0) {
        if(!textureID) {
            createTexture(width, height, frame.image.data);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frame.image.data);
        }
    }
}
