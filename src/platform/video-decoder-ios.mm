#include "video-decoder-ios.hpp"
#include "ccplus.hpp"
#import <Foundation/Foundation.h>

@interface VDecoder : NSObject

-(id)initWithURL:(const char*)url;
-(void)seekTo:(float)time;
-(float)duration;
-(int)width;
-(int)height;
-(float)decodeImage;
-(void)getDecodedImage:(ResultVideoFrameCallback)callback inCtx:(void*)ctx;
-(void)decodeAudio:(float)length callback:(ResultAudioCallback)callback inCtx:(void*)ctx;

@end

using namespace CCPlus;

VideoDecoderIOS::VideoDecoderIOS(const std::string& inputFile) {
    VDecoder* ctx = [[VDecoder alloc]initWithURL:inputFile.c_str()];
    decodeContext = CFBridgingRetain(ctx);
}

VideoDecoderIOS::~VideoDecoderIOS() {
    CFBridgingRelease(decodeContext);
}

VideoInfo VideoDecoderIOS::getVideoInfo() {
    VideoInfo ret;
    VDecoder* ctx = CFBridgingRelease(decodeContext);
    ret.duration = [ctx duration];
    ret.width = ret.rwidth = [ctx width];
    ret.height = ret.rheight = [ctx height];
    decodeContext = CFBridgingRetain(ctx);
    return ret;
}

void VideoDecoderIOS::seekTo(float time, bool realSeek) {
    VDecoder* ctx = CFBridgingRelease(decodeContext);
    [ctx seekTo:time];
    decodeContext = CFBridgingRetain(ctx);
}

float VideoDecoderIOS::decodeImage() {
    VDecoder* ctx = CFBridgingRelease(decodeContext);
    float ret = [ctx decodeImage];
    decodeContext = CFBridgingRetain(ctx);
    return ret;
}

void setDecodedImage(void* ctx, const char* bgra, int width, int height, int bytesPerRow, int rotation) {
    Frame& frame = *((Frame*)((void**)ctx)[0]);
    int& maxSize = *((int*)((void**)ctx)[1]);
    frame.image = cv::Mat(height, width, CV_8UC4);
    uint8_t* tdata = frame.image.data;
    int tbytesPerRow = width * 4;
    for(int i = 0; i < height; i++) {
        memcpy(tdata + i * tbytesPerRow, bgra + i * bytesPerRow, tbytesPerRow);
    }
    frame.toNearestPOT(maxSize, renderMode == PREVIEW_MODE);
    if (rotation == 180) {
        flip(frame.image, frame.image, -1); 
    } else if (rotation == 90) {
        transpose(frame.image, frame.image);
        flip(frame.image, frame.image, 1); 
    } else if (rotation == 270) {
        transpose(frame.image, frame.image);
        flip(frame.image, frame.image, 0); 
    }
}
Frame VideoDecoderIOS::getDecodedImage(int maxSize) {
    Frame ret;
    void* ictx[2] = {&ret, &maxSize};
    VDecoder* ctx = CFBridgingRelease(decodeContext);
    [ctx getDecodedImage:setDecodedImage inCtx:ictx];
    decodeContext = CFBridgingRetain(ctx);
    if(!ret.image.empty()) {
        ret.ext.scaleAdjustX = getVideoInfo().width * 1.0 / ret.image.cols;
        ret.ext.scaleAdjustY = getVideoInfo().height * 1.0 / ret.image.rows;
    }
    return ret;
}

void setDecodedAudio(void* ctx, const char* audio, int len, float time) {
    float& stime = *((float*)((void**)ctx)[0]);
    std::vector<int16_t>& vec = *((std::vector<int16_t>*)((void**)ctx)[1]);
    if(stime > time)
        stime = time;
    vec.reserve(vec.size() + len / 2);
    vec.insert(vec.end(), (int16_t*)audio, (int16_t*)(audio + len));
}
float VideoDecoderIOS::decodeAudio(std::vector<int16_t>& vec, float durationLimit) {
    float ret = 99999;
    void* ictx[2] = {&ret, &vec};
    VDecoder* ctx = CFBridgingRelease(decodeContext);
    [ctx decodeAudio:durationLimit callback:setDecodedAudio inCtx:ictx];
    decodeContext = CFBridgingRetain(ctx);
    return ret;
}
