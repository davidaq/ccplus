#ifdef __ANDROID__

#include "global.hpp"
#include "platform.hpp"

extern "C" {
    void jniCCPlusCreateGLContext();
    void jniCCPlusDestroyGLContext();
    void jniCCPlusCopyAssets();
}

void* CCPlus::createGLContext() {
    jniCCPlusCreateGLContext();
    return 0;
}

void CCPlus::destroyGLContext(void*) {
    jniCCPlusDestroyGLContext();
}

void CCPlus::copyAssets() {
    jniCCPlusCopyAssets();
}

#endif
