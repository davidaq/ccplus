#ifdef __ANDROID__

#include "global.hpp"
#include "platform.hpp"

extern "C" {
    void jniCCPlusCreateGLContext();
    void jniCCPlusDestroyGLContext();
    void jniCCPlusReadAsset(const char*, void**, int* sz);
}

void* CCPlus::createGLContext() {
    jniCCPlusCreateGLContext();
    return 0;
}

void CCPlus::destroyGLContext(void*) {
    jniCCPlusDestroyGLContext();
}

cv::Mat CCPlus::readAsset(const char* name) {
    void* data = 0;
    int sz;
    jniCCPlusReadAsset(name, &data, &sz);
    if(data && sz > 0) {
        cv::Mat ret(1, sz, CV_8U);
        memcpy(ret.data, data, sz);
        free(data);
        return ret;
    } else {
        return cv::Mat();
    }
}

#endif
