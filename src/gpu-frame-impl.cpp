#include "gpu-frame-impl.hpp"
#include "frame.hpp"
#include "profile.hpp"
#include "gpu-frame-cache.hpp"

using namespace CCPlus;

GPUFrameImpl::~GPUFrameImpl() {
    GPUFrameCache::reuse(this);
}

void GPUFrameImpl::bindFBO(bool clear) {
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    glViewport(0, 0, width, height);

    if (clear) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

Frame GPUFrameImpl::toCPU() {
    Frame ret;
    ret.ext = ext;
    if(textureID) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        ret.image = cv::Mat::zeros(height, width, CV_8UC4);
        bindFBO();
        glReadPixels(0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, ret.image.data);
        glFinish();
    }
    return ret;
}

void GPUFrameImpl::load(const Frame& frame) {
    ext = frame.ext;
    if(width * height > 0) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, 
                GL_BGRA_EXT, GL_UNSIGNED_BYTE, frame.image.data);
    }
}
