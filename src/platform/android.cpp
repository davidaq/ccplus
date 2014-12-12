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

#endif
