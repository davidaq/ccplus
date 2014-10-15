#include "text-renderable.hpp"
#include "frame.hpp"
#include "gpu-frame.hpp"
#include "logger.hpp"
#include "context.hpp"
#include <algorithm>

using namespace cv;
using namespace CCPlus;
using namespace std;

int TextRenderable::getWidth() const {
    int n = 0;
    for (auto& kv : text) {
        n = max<int>(kv.second.length(), n);
    }
    return n * getHeight();
}

int TextRenderable::getHeight() const {
    int ret = 0;
    for (auto& kv : size) {
        ret = max<int>(ret, kv.second);
    }
    return ret;
}

float TextRenderable::getDuration() {
    return 0xffffff;
}

void TextRenderable::prepare() {
    // Generate key frame list
    auto f = [&] (int t) {
        keyframes.push_back(t);
    };
    for (auto& kv : text)  f(kv.first);
    for (auto& kv : font)  f(kv.first);
    for (auto& kv : size)  f(kv.first);
    for (auto& kv : scale_x)  f(kv.first);
    for (auto& kv : scale_y)  f(kv.first);
    for (auto& kv : tracking)  f(kv.first);
    for (auto& kv : bold)  f(kv.first);
    for (auto& kv : italic)  f(kv.first);
    std::unique(keyframes.begin(), keyframes.end());
    std::sort(keyframes.begin(), keyframes.end());
    if (keyframes.empty()) {
        log(logWARN) << "TextRenderable hasn't been given parameters";
    }
    // prepare only key frames
    for(int f : keyframes) {
        prepareFrame(f);
    }
}

void TextRenderable::updateGPUFrame(GPUFrame& frame, float time) {
    int kTime = findKeyTime(time);
    if(frame.tag != kTime) {
        L() << kTime;
        frame.tag = kTime;
        Frame cframe = framesCache[kTime];
        frame.load(cframe);
    }
}

void TextRenderable::release() {
    framesCache.clear();
    keyframes.clear();
}

void TextRenderable::prepareFrame(int itime) {
    float time = IF(itime);
    if(framesCache.count(itime))
        return;
    int width = getWidth();
    int height = getHeight();

    FT_Face& face = Context::getContext()->fontFace();
    int error;

    int size = get(this->size, time);
    float sx = get<float>(this->scale_x, time);
    float sy = get<float>(this->scale_y, time);
    if (FT_Set_Pixel_Sizes(face, (int)size * sx, (int)size * sy)) {
        if (FT_Set_Pixel_Sizes(face, (int)size, (int)size)) {
            if (FT_Set_Pixel_Sizes(face, 20, 20)) {
                log(logFATAL) << "Can't set text size";
            }
        }
    }
    int color = get(this->color, time);
    int r = (color >> 14) * 255 / 127;
    int g = ((color >> 7) & 127) * 255 / 127;
    int b = (color & 127) * 255 / 127;
    float tracking = get<float>(this->tracking, time);
    Mat ret((int)height * 2 * sy, 
            (int)width * (2 + tracking) * sx,
            CV_8UC4, cv::Scalar(b, g, r, 0));
    auto draw = [&ret, width, height] (FT_Bitmap* bitmap, int sx, int sy) {
        int rows = bitmap->rows;
        int cols = bitmap->width;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int ty = sy + i;
                int tx = sx + j;
                if (ty >= 2 * height || tx >= 2 * width || 
                    ty < 0 || tx < 0) 
                    continue;
                if (bitmap->buffer[i * cols + j] == 0)
                    continue;
                ret.at<Vec4b>(ty, tx)[3] =
                    bitmap->buffer[i * cols + j]; 
            }
        }
    };

    wstring s = get<wstring>(this->text, time);
    int x = 0;
    int y = height * sy;
    int prevAdvance = 0;
    for (int j = 0; j < s.length(); j++) {
        error = FT_Load_Char(face, s[j], FT_LOAD_RENDER);
        if (error) {
            log(logWARN) << "Can't load character: " << s[j];
            x += size * 1.5;
        } else {
            x += prevAdvance;
            FT_GlyphSlot slot = face->glyph;
            draw(&slot->bitmap, slot->bitmap_left + x, y - slot->bitmap_top);
            float advance = (slot->advance.x >> 6) - slot->bitmap.width;
            if(advance < 0.5)
                advance = 0.5;
            x += slot->bitmap.width;
            prevAdvance = (1 + tracking) * advance;
        }
    }
    Frame retFrame;
    retFrame.image = ret;
    retFrame.ext.anchorAdjustY = size;
    switch(get(this->justification, time)) {
        case 0: // left
            retFrame.ext.anchorAdjustX = 0;
            break;
        case 1: // center
            retFrame.ext.anchorAdjustX = x / 2;
            break;
        case 2: // right
            retFrame.ext.anchorAdjustX = x;
            break;
    };
    framesCache[itime] = retFrame;
}

int TextRenderable::findKeyTime(float time) {
    int itime = FI(time);
    int ret = itime;
    for (int key : keyframes) {
        if (key <= itime) 
            ret = key;
        if(key > itime)
            break;
    }
    return ret;
}
