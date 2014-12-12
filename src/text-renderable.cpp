#include "text-renderable.hpp"
#include "frame.hpp"
#include "gpu-frame.hpp"
#include "logger.hpp"
#include "context.hpp"
#include <algorithm>
#include <locale>

using namespace cv;
using namespace CCPlus;
using namespace std;

static inline int FI(float v) {
    return (int)(v * 1000);
}

TextRenderable::TextRenderable(const boost::property_tree::ptree& tree) {
    auto each = [&tree] (const std::string& name, 
            std::function<void(int, const std::string&)> f,
            const std::string& defVal) {
        try {
            for (auto& pc : tree.get_child("text-properties." + name)) {
                float t = std::atof(pc.first.data());
                f(FI(t), pc.second.data());
            }
        } catch(...) {
            f(0, defVal);
        }
    };
    each("text", [&] (int t, const std::string& pc) {
        utf8toWStr(text[t], pc);
    }, "");
    each("font", [&] (int t, const std::string& pc) {
        font[t] = pc + ".ttf";
        stringToLower(font[t]);
    }, "Arial");
    each("size", [&] (int t, const std::string& pc) {
        size[t] = std::atoi(pc.c_str());
    }, "20");
    each("tracking", [&] (int t, const std::string& pc) {
        tracking[t] = std::atof(pc.c_str());
    }, "0");
    each("bold", [&] (int t, const std::string& pc) {
        bold[t] = (pc[0] == 't');
    }, "false");
    each("italic", [&] (int t, const std::string& pc) {
        italic[t] = (pc[0] == 't');
    }, "false");
    each("scale_x", [&] (int t, const std::string& pc) {
        scale_x[t] = std::atof(pc.c_str());
    }, "1");
    each("scale_y", [&] (int t, const std::string& pc) {
        scale_y[t] = std::atof(pc.c_str());
    }, "1");
    each("color", [&] (int t, const std::string& pc) {
        color[t] = std::atoi(pc.c_str());
    }, "2097151");
    each("justification", [&] (int t, const std::string& s) {
        justification[t] = std::atoi(s.c_str());
    }, "0");
}

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

GPUFrame TextRenderable::getGPUFrame(float time) {
    int kTime = findKeyTime(time);
    if(!gpuFramesCache.count(kTime)) {
        if(!framesCache.count(kTime))
            return GPUFrame();
        gpuFramesCache[kTime] = framesCache[kTime].toGPU();
    }
    return gpuFramesCache[kTime];
}

void TextRenderable::release() {
    framesCache.clear();
    keyframes.clear();
    gpuFramesCache.clear();
}

void TextRenderable::prepare() {
    static Lock lock;
    // Generate key frame list
    lock.lock();
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
    lock.unlock();
}

void TextRenderable::prepareFrame(int time) {
    if(framesCache.count(time))
        return;
    int width = getWidth();
    int height = getHeight();

    std::string fontName = get<std::string>(this->font, time);
    cv::Mat fontData = readAsset(fontName.c_str());
    if(fontData.empty())
        fontData = readAsset(("fonts/" + fontName).c_str());
    if(fontData.empty())
        fontData = readAsset("font.ttf");
    if(fontData.empty())
        fontData = readAsset("default.ttf");
    if(fontData.empty()) {
        log(logFATAL) << "Can't find font nor a default one...";
        return;
    }
    FT_Face face;
    int error;
    error = FT_New_Memory_Face(Context::getContext()->freetype(),
            (const unsigned char*) fontData.data, fontData.total(), 0, &face);
    if(error) {
        log(logFATAL) << "Can't load font...";
        return;
    }

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
                if (ty >= ret.rows || tx >= ret.cols || 
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
    // Calculate correct anchor
    for (int j = 0; j < s.length(); j++) {
        error = FT_Load_Char(face, s[j], FT_LOAD_RENDER);
        if (error) {
            log(logWARN) << "Can't load character: " << s[j];
            x += size * 1.5;
        } else {
            x += prevAdvance;
            FT_GlyphSlot slot = face->glyph;
            int bearing = slot->metrics.horiBearingY / 72;
            //L() << bearing << y - bearing << "~" << y - bearing + slot->bitmap.rows;
            //draw(&slot->bitmap, slot->bitmap_left + x, y - slot->bitmap_top);
            draw(&slot->bitmap, slot->bitmap_left + x, y - bearing);
            float advance = (slot->advance.x >> 6) - slot->bitmap.width;
            if(advance < 0.5)
                advance = 0.5;
            x += slot->bitmap.width;
            prevAdvance = (1 + tracking) * advance;
        }
    }
    Frame retFrame;
    retFrame.image = ret;
    //retFrame.ext.anchorAdjustY = y / 1.5;
    retFrame.ext.anchorAdjustY = y - 6;
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
    retFrame.toNearestPOT(1024);
#ifdef __ANDROID__
    if(!retFrame.image.empty())
        cv::cvtColor(retFrame.image, retFrame.image, CV_BGRA2RGBA);
#endif
    framesCache[time] = retFrame;
    FT_Done_Face(face);
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
