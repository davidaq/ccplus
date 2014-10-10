#ifdef __OSX__

#include "externals/gl2.h"
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>

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
    errorCode = CGLCreateContext( pix, NULL, &ctx  ); // second parameter can be another context for object sharing
    CGLDestroyPixelFormat( pix  );

    errorCode = CGLSetCurrentContext( ctx  );
    //printf("OpenGL version: %s\n", glGetString(GL_VERSION));
}

#endif
