#include "gif-renderable.hpp"
#include "context.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "externals/gif_lib.h"
#include "gpu-frame.hpp"

using namespace CCPlus;

GifRenderable::GifRenderable(const std::string& uri) {
    path = parseUri2File(uri);
}

void GifRenderable::release() {
    framesCache.clear();
}

GPUFrame GifRenderable::getGPUFrame(float time) {
    if(framesCache.empty())
        return GPUFrame();
    int index = 0;
    for(int i = 0; i < framesCache.size(); i++) {
        if(framesCache[i].first <= time)
            index = i;
        else
            break;
    }
    Frame f;
    f.readZimCompressed(framesCache[index].second);
    GPUFrame ret = GPUFrameCache::alloc(f.image.cols, f.image.rows);
    ret->load(f);
    return ret;
}

float GifRenderable::getDuration() {
    return duration;
}

struct GifPallete {
    int transparent = 1;
    int disposeMode = 1;
    int interlace = -1;
    float delay = -1;
};

template<typename T>
GifPallete readGifExt(const T& src, GifPallete defVal) {
    GifPallete ret = defVal;
    for(int i = 0; i < src->ExtensionBlockCount; i++) {
        auto& ext = src->ExtensionBlocks[i];
        if(ext.Function == GRAPHICS_EXT_FUNC_CODE) {
            unsigned char* data = (unsigned char*) ext.Bytes;
            if(data[0] & 1) {
                ret.transparent = (0x100 + data[3]) & 0xff;
            } else {
                ret.transparent = -1;
            }
            ret.delay = (data[1] + (data[2] << 8)) * 0.01;
            if(ret.delay < 0.01)
                ret.delay = 0.04;
            ret.disposeMode = (data[0] >> 2) & 7;
        }
    }
    return ret;
}

void GifRenderable::prepare() {
    static Lock gifPrepareLock;
    gifPrepareLock.lock();
    prepareGif();
    gifPrepareLock.unlock();
}

void GifRenderable::prepareGif() {
    int error;
    GifFileType* ctx = DGifOpenFileName(path.c_str(), &error);
    log(logINFO) << "GIF open [" << error << "]: " << path;
    error = DGifSlurp(ctx);
    log(logINFO) << "GIF slurp [" << error << "]: " << path;
    if(error != D_GIF_SUCCEEDED && error != 1)
        return;

    GifPallete global = readGifExt(ctx, GifPallete());

    cv::Mat im, prev;
    float currentTime = 0;
    for(int frame = 0; frame < ctx->ImageCount; frame++) {
        if(!im.empty())
            prev = im.clone();
        SavedImage &cGifFrame = ctx->SavedImages[frame];
        ColorMapObject* map = ctx->SColorMap;
        if(cGifFrame.ImageDesc.ColorMap) {
            map = cGifFrame.ImageDesc.ColorMap;
        }
        if(!map) continue;
        GifPallete pallete = readGifExt(&cGifFrame, global);
        if(im.empty()) {
            cv::Scalar background = cv::Scalar(0, 0, 0, 0);
            if(pallete.transparent != ctx->SBackGroundColor && ctx->SBackGroundColor < map->ColorCount) {
                auto& color = map->Colors[pallete.transparent];
                background = cv::Scalar(color.Blue, color.Green, color.Red, 255);
            }
            im = cv::Mat(ctx->SHeight, ctx->SWidth, CV_8UC4, background);
        }
        
        const int cols = im.cols;
        cv::Vec4b* dst = im.ptr<cv::Vec4b>(0);
        int i = 0;
        auto scan = [&i, dst, &cGifFrame, &pallete, cols, map](int y) {
            for(int x = 0; x < cGifFrame.ImageDesc.Width; x++) {
                i++;
                int colorIndex = cGifFrame.RasterBits[i];
                if(colorIndex == pallete.transparent) {
                    continue;
                }
                GifColorType rgb = map->Colors[colorIndex];
                int px = cGifFrame.ImageDesc.Left + x;
                int py = cGifFrame.ImageDesc.Top + y;
                int p = py * cols + px;
                dst[p][0] = rgb.Blue;
                dst[p][1] = rgb.Green;
                dst[p][2] = rgb.Red;
                dst[p][3] = 0xff;
            }
        };
        if(cGifFrame.ImageDesc.Interlace) {
            static const int InterlacedOffset[] = {0, 4, 2, 1}, InterlacedJumps[] = {8, 8, 4, 2};
            for(int pass = 0; pass < 4; pass++) {
                for(int y = InterlacedOffset[pass]; y < cGifFrame.ImageDesc.Height;
                        y += InterlacedJumps[pass]) {
                    scan(y);
                }
            }
        } else {
            for(int y = 0; y < cGifFrame.ImageDesc.Height; y++) {
                scan(y);
            }
        }
        Frame ret;
        ret.image = im;
        ret.toNearestPOT(256);
#ifdef __ANDROID__
        if(!ret.image.empty())
            cv::cvtColor(ret.image, ret.image, CV_BGRA2RGBA);
#endif
        framesCache.push_back(std::pair<float, cv::Mat>(currentTime, ret.zimCompressed()));

        currentTime += pallete.delay;
        // only allow gif less than 10 sec
        if(currentTime > 10)
            break;
        switch(pallete.disposeMode) {
            case 1:
                // draw on top
                break;
            case 3:
                // restore
                im = prev.clone();
                break;
            default:
                // background
                im = cv::Mat();
                break;
        }
    }
    duration = currentTime + 0.05;
    DGifCloseFile(ctx, &error);
}

