#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"

using namespace cv;
using namespace CCPlus;
using namespace boost;

std::map<CCPlus::GPUFrameCache::Size, std::vector<std::pair<GLuint, GLuint>>> GPUFrameCache::cache;

GPUFrame GPUFrameCache::alloc(int width, int height) {
    if (width == 0 && height == 0) {
        return boost::shared_ptr<GPUFrameImpl>(new GPUFrameImpl()); 
    }
    auto* p = &cache[Size(width, height)];
    int sz = p->size();
    if (sz > 0) {
        GPUFrameImpl* frame = new GPUFrameImpl();
        frame->width = width;
        frame->height = height;
        frame->textureID = (*p)[sz - 1].first;
        frame->fboID = (*p)[sz - 1].second;
        p->pop_back();
        return boost::shared_ptr<GPUFrameImpl>(frame); 
    } else {
        GPUFrameImpl* frame = new GPUFrameImpl();
        frame->width = width;
        frame->height = height;
        
        glGenTextures(1, &frame->textureID);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                GL_BGRA_EXT, GL_UNSIGNED_BYTE, 0);

        glGenFramebuffers(1, &frame->fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, frame->fboID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_2D, frame->textureID, 0);
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return boost::shared_ptr<GPUFrameImpl>(frame); 
    }
}

void GPUFrameCache::reuse(GPUFrameImpl* frame) {
    cache[Size(frame->width, frame->height)].push_back({frame->textureID, frame->fboID});
}

void GPUFrameCache::clear() {
    cache.clear();
}
