#ifdef __IOS__

#include "stdio.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#import <GLKit/GLKit.h>
#include "platform.hpp"

void* CCPlus::createGLContext() {
    EAGLContext* glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];
}

void CCPlus::destroyGLContext(void*) {
    //[EAGLContext setCurrentContext:nil];
}

#endif
