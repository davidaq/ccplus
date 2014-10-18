#ifdef __IOS__

#include "stdio.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#import <GLKit/GLKit.h>
#include "platform.hpp"

void CCPlus::createGLContext() {
    EAGLContext* glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];
}

cv::Mat CCPlus::readAsset(const char* name) {
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* bundlePath = [bundle pathForResource:@"ccplus"
                                            ofType:@"bundle"];
    NSLog(@"Searched ccplus.bundle path %@", bundlePath);
    if(!bundlePath) {
        NSLog(@"Assets bundle ccplus.bundle not found");
        return cv::Mat();
    }
    bundle = [NSBundle bundleWithPath:bundlePath];
    NSString* resPath = [NSString stringWithUTF8String:name];
    resPath = [bundle pathForResource:resPath ofType:@"f"];

    FILE* fp = fopen([resPath UTF8String], "rb");
    if(!fp) {
        printf("------FILE: %s\n", name);
        NSLog(@"Asset %@ not found", resPath);
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
