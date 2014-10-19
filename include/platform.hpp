#pragma once

namespace CCPlus {
    void* createGLContext();
    void destroyGLContext(void*);
    cv::Mat readAsset(const char* name);
}
