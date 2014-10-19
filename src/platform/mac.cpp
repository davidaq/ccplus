#ifdef __OSX__

#include "global.hpp"
#include <OpenGL/OpenGl.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>

using namespace CCPlus;

void* CCPlus::createGLContext() {
    /* OpenGL initialization: only for testing */
    CGLContextObj* ret = new CGLContextObj;
    CGLContextObj& ctx = *ret;
    CGLPixelFormatAttribute attributes[4] = {
        kCGLPFAAccelerated,   
        kCGLPFAOpenGLProfile, 
        (CGLPixelFormatAttribute) kCGLOGLPVersion_Legacy,
        (CGLPixelFormatAttribute) 0

    };
    CGLPixelFormatObj pix;
    CGLError errorCode;
    GLint num; // stores the number of possible pixel formats
    errorCode = CGLChoosePixelFormat(attributes, &pix, &num);
    errorCode = CGLCreateContext(pix, nullptr, &ctx); // second parameter can be another context for object sharing
    CGLDestroyPixelFormat(pix);

    errorCode = CGLSetCurrentContext(ctx);
    if (errorCode) {
        log(logFATAL) << "OpenGL initializtion failed, code: " << errorCode;
    }
    return ret;
}

void CCPlus::destroyGLContext(void* ctx) {
    CGLSetCurrentContext(0);
    CGLDestroyContext(*((CGLContextObj*)ctx));
}

cv::Mat CCPlus::readAsset(const char* _name) {
    std::string name = "assets/";
    name += _name;
    FILE* fp = fopen(name.c_str(), "rb");
    if(!fp) {
        log(logWARN) << "Asset " + name + " not found";
        return cv::Mat();
    }
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    cv::Mat ret(1, len, CV_8U);
    fread(ret.data, 1, len, fp);
    fclose(fp);
    return ret;
}

#endif
