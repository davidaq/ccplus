#include "gif-renderable.hpp"
#include "context.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "../giflib/gif_lib.h"

using namespace CCPlus;

GifRenderable::GifRenderable(CCPlus::Context* context, const std::string& _uri):
    AnimatedRenderable(context), uri(_uri)
{
    std::string path = uri;
    if (stringStartsWith(path, "file://")) 
        path = uri.substr(7);
    path = generatePath(context->getInputDir(), path);
    int error;
    ctx = DGifOpenFileName(path.c_str(), &error);
    L() << "GIF open [" << error << "]: " << path;
    DGifSlurp(ctx);
    if(error != D_GIF_SUCCEEDED && error != 1)
        ctx = 0;
}

GifRenderable::~GifRenderable() {
    int error;
    if(ctx)
        DGifCloseFile(ctx, &error);
}

#define INIT(...) if(!ctx) return __VA_ARGS__;

int GifRenderable::getWidth() const {
    INIT(0);
    return ctx->SWidth;
}

int GifRenderable::getHeight() const {
    INIT(0);
    return ctx->SHeight;
}

const std::string& GifRenderable::getName() const {
    return uri;
}

float GifRenderable::getDuration() const {
    INIT(1.0f);
    return ctx->ImageCount * 1.0f / GIF_FPS;
}

bool GifRenderable::still(float t1, float t2) {
    INIT(true);
    return false;
}

void GifRenderable::renderPart(float start, float duration) {
    INIT();
    float step = 1.f / context->getFPS();
    for(float t = start, l = start + duration; t < l; t += step) {
        int frameNum = getFrameNumber(t);
        if(!rendered.count(frameNum)) {
            int gifFrame = t * GIF_FPS + 0.5f;
            gifFrame %= ctx->ImageCount;
            cv::Mat im = cv::Mat(getHeight(), getWidth(), CV_8UC1, ctx->SavedImages[gifFrame].RasterBits);
            cv::cvtColor(im, im, CV_GRAY2BGRA);
            Frame frame(im);
            std::string fileName = getFramePath(frameNum);
            frame.write(fileName);
            rendered.insert(frameNum);
        }
    }
}

