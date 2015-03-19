#ifdef NEVER_DEFINED

#include "video-encoder.hpp"
#include "frame.hpp"
#import <Foundation/Foundation.h>

@interface MediaEncoder : NSObject

-(id)initWithFilePath:(const char*)cpath fps:(int)fps_ width:(int)width height:(int)height;
-(void)appendFrame:(const char*)bgra width:(int)width height:(int)height audio:(const int16_t*)audio audioLen:(int)audioLen;
-(void)finish;

@end

using namespace CCPlus;

VideoEncoder::VideoEncoder(const std::string& outputPath, int fps, int width, int height, int quality) {
    MediaEncoder* encoder = [[MediaEncoder alloc]initWithFilePath:outputPath.c_str() fps:fps width:width height:height];
    encodeContext = CFBridgingRetain(encoder);
}

VideoEncoder::~VideoEncoder() {
    CFBridgingRelease(encodeContext);
}

void VideoEncoder::appendFrame(const CCPlus::Frame& frame) {
    MediaEncoder* encoder = CFBridgingRelease(encodeContext);
    [encoder appendFrame:(const char*)frame.image.data width:frame.image.cols height:frame.image.rows audio:(const int16_t*)frame.ext.audio.data audioLen:frame.ext.audio.total()];
    encodeContext = CFBridgingRetain(encoder);
}

void VideoEncoder::finish() {
    MediaEncoder* encoder = CFBridgingRelease(encodeContext);
    [encoder finish];
    encodeContext = CFBridgingRetain(encoder);
}

#endif
