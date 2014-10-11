#ifdef __OSX__

#include "platform.hpp"
#include "externals/gl2.h"
#include <OpenGL/OpenGl.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>

using namespace CCPlus;

void createGLContext() {
    /* OpenGL initialization: only for testing */
    CGLContextObj ctx;
    CGLPixelFormatAttribute attributes[4] = {
        kCGLPFAAccelerated,   
        kCGLPFAOpenGLProfile, 
        (CGLPixelFormatAttribute) kCGLOGLPVersion_Legacy,
        (CGLPixelFormatAttribute) 0

    };
    CGLPixelFormatObj pix;
    CGLError errorCode;
    GLint num; // stores the number of possible pixel formats
    errorCode = CGLChoosePixelFormat( attributes, &pix, &num  );
    errorCode = CGLCreateContext( pix, nullptr, &ctx  ); // second parameter can be another context for object sharing
    CGLDestroyPixelFormat( pix  );

    errorCode = CGLSetCurrentContext( ctx  );
    //printf("OpenGL version: %s\n", glGetString(GL_VERSION));
}

cv::Mat readAsset(std::string name) {
    name = "assets/" + name;
    FILE* fp = fopen(name.c_str(), "rb");
    if(!fp) {
        log(logWARN) << "Asset " + name + " not found";
        return cv::Mat();
    }
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    cv::Mat ret = cv::Mat(1, len, CV_8U);
    fread(ret.data, 1, len, fp);
    return ret;
}

#endif
