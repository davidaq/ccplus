#include "gif-renderable.hpp"
#include "context.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "externals/gif_lib.h"

using namespace CCPlus;

GifRenderable::GifRenderable(const std::string& _uri) {
    path = Context::getContext()->getFootagePath(path);
}

void GifRenderable::release() {
    framesCache.clear();
    gpuFramesCache.clear();
}

float GifRenderable::getDuration() {
    return 0xffffff;
}

void GifRenderable::prepare() {
    //int error;
    //GifFileType* ctx = DGifOpenFileName(path.c_str(), &error);
    //log(logINFO) << "GIF open [" << error << "]: " << path;
    //error = DGifSlurp(ctx);
    //log(logINFO) << "GIF slurp [" << error << "]: " << path;
    //if(error != D_GIF_SUCCEEDED && error != 1)
    //    return;
    //// always render 0 - 7s
    //start = 0;
    //if(duration > 7)
    //    duration = 7;

    //float step = 1.f / Context::getContext()->fps;
    //static const int InterlacedOffset[] = {0, 4, 2, 1}, InterlacedJumps[] = {8, 8, 4, 2};
    //int gifFPS = GIF_FPS;
    //int gTransparent = -1;
    //for(int i = 0; i < ctx->ExtensionBlockCount; i++) {
    //    auto& ext = ctx->ExtensionBlocks[i];
    //    if(ext.Function == GRAPHICS_EXT_FUNC_CODE) {
    //        auto data = ext.Bytes;
    //        //gifFPS = data[4];
    //        if(data[0] & 1) {
    //            gTransparent = (0x100 + data[3]) & 0xff;
    //        } else {
    //            gTransparent = -1;
    //        }
    //    }
    //}
    //ColorMapObject* globalMap = ctx->SColorMap;

    //        gifFrame %= ctx->ImageCount;
    //        SavedImage &src = ctx->SavedImages[gifFrame];
    //cv::Mat im;
    //
    //int disposeMode = 0;
    //for(int i = 0; i < src.ExtensionBlockCount; i++) {
    //    auto& ext = src.ExtensionBlocks[i];
    //    if(ext.Function == GRAPHICS_EXT_FUNC_CODE) {
    //        uint8_t* data = (uint8_t*)ext.Bytes;
    //        //gifFPS = data[4];
    //        if(data[0] & 1) {
    //            transparent = (0x100 + data[3]) & 0xff;
    //        } else {
    //            transparent = -1;
    //        }
    //        disposeMode = (data[0] >> 2) & 7;
    //    }
    //}

    //ColorMapObject* map = globalMap;
    //if(src.ImageDesc.ColorMap) {
    //    map = src.ImageDesc.ColorMap;
    //}
    //if(im.empty()) {
    //    cv::Scalar background = cv::Scalar(0, 0, 0, 0);
    //    if(transparent != ctx->SBackGroundColor && map && ctx->SBackGroundColor < map->ColorCount) {
    //        auto& color = map->Colors[transparent];
    //        background = cv::Scalar(color.Blue, color.Green, color.Red, 255);
    //    }
    //    im = cv::Mat(getHeight(), getWidth(), CV_8UC4, background);
    //}
    //int i = 0;
    //const int cols = im.cols;
    //cv::Vec4b* dst = im.ptr<cv::Vec4b>(0);
    //auto scan = [dst, &src, &i, cols, map, transparent, gTransparent](int y) {
    //    for(int x = 0; x < src.ImageDesc.Width; x++) {
    //        i++;
    //        int colorIndex = src.RasterBits[i];
    //        if(colorIndex == transparent || colorIndex == gTransparent) {
    //            continue;
    //        }
    //        GifColorType rgb;
    //        if(map) {
    //            rgb = map->Colors[colorIndex];
    //        } else {
    //            break;
    //        }
    //        int px = src.ImageDesc.Left + x;
    //        int py = src.ImageDesc.Top + y;
    //        int p = py * cols + px;
    //        dst[p][0] = rgb.Blue;
    //        dst[p][1] = rgb.Green;
    //        dst[p][2] = rgb.Red;
    //        dst[p][3] = 0xff;
    //    }
    //};
    //if(src.ImageDesc.Interlace) {
    //    for(int pass = 0; pass < 4; pass++) {
    //        for(int y = InterlacedOffset[pass]; y < src.ImageDesc.Height; y+= InterlacedJumps[pass]) {
    //            scan(y);
    //        }
    //    }
    //} else {
    //    for(int y = 0; y < src.ImageDesc.Height; y++) {
    //        scan(y);
    //    }
    //}
    //        SavedImage &src = ctx->SavedImages[gifFrame];

    //framesCache

    //switch(disposeMode) {
    //    case 1:
    //        // draw on top
    //        prev = im.clone();
    //        break;
    //    case 3:
    //        // restore
    //        im = prev.clone();
    //        break;
    //    default:
    //        // background
    //        prev = im;
    //        im = cv::Mat();
    //        break;
    //}

    //DGifCloseFile(ctx, &error);
}

