#ifdef __IOS__

#include "stdio.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#import <GLKit/GLKit.h>
#include "platform.hpp"

void* CCPlus::createGLContext() {
    EAGLContext* glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];
    return 0;
}

void CCPlus::destroyGLContext(void*) {
    [EAGLContext setCurrentContext:nil];
}

void CCPlus::removeFile(const std::string& fileName, bool removeDir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* fname = [NSString stringWithUTF8String:fileName.c_str()];
    BOOL isDir;
    if([fm fileExistsAtPath:fname isDirectory:&isDir]) {
        if(removeDir || !isDir) {
            [fm removeItemAtPath:fname error:nil];
        }
    }
}

#endif
