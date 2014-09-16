#include "text-renderable.hpp"
#include "frame.hpp"
#include "logger.hpp"
#include "extra-context.hpp"
#include <algorithm>

using namespace cv;
using namespace CCPlus;
using namespace std;

TextRenderable::TextRenderable(Context* context, 
        const std::string& _uri) 
    : Renderable(context), uri(_uri) {
    
} 

const std::string& TextRenderable::getName() const {
    return uri;
}

void TextRenderable::clear() {
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

float TextRenderable::getDuration() const {
    return 0x7fffffff;
}

bool TextRenderable::still(float t1, float t2) {
    return findKeyByTime(t1) == findKeyByTime(t2);
}

Frame TextRenderable::getFrame(float time) const {
    return getFrameByNumber(getFrameNumber(time));
}

Frame TextRenderable::getFrameByNumber(int frame) const {
    if (keyframes.empty()) {
        log(logWARN) << "Access to a unrendered text object";
        return Frame();
    }
    int retf = findKey(frame);
    if (rendered.count(retf))
        return Frame(getFramePath(retf));
    return Frame();
}

void TextRenderable::render(float start, float duration) {
    // Generate key frame list
    if (keyframes.empty()) {
        auto f = [&] (float t) {
            keyframes.push_back(getFrameNumber(t));
        };
        for (auto& kv : text)  f(kv.first);
        for (auto& kv : font)  f(kv.first);
        for (auto& kv : size)  f(kv.first);
        for (auto& kv : scale_x)  f(kv.first);
        for (auto& kv : scale_y)  f(kv.first);
        for (auto& kv : tracking)  f(kv.first);
        for (auto& kv : bold)  f(kv.first);
        for (auto& kv : italic)  f(kv.first);
        sort(keyframes.begin(), keyframes.end());
        unique(keyframes.begin(), keyframes.end());
        if (keyframes.empty()) {
            log(logWARN) << "TextRenderable hasn't been given parameters";
        }
    }

    int startFrame = getFrameNumber(start);
    int endFrame = getFrameNumber(start + duration);
    int width = getWidth();
    int height = getHeight();

    FT_Face& face = context->getExtra().font;
    int error;

    for (int i = startFrame; i <= endFrame; i++) {
        // Come on FreeType!
        float time = getFrameTime(i);
        int size = get(this->size, time);
        float sx = get<float>(this->scale_x, time);
        float sy = get<float>(this->scale_y, time);
        if (FT_Set_Pixel_Sizes(face, (int)size * sx, (int)size * sy)) {
            log(logFATAL) << "Can't set text size";
        }
        int color = get(this->color, time);
        int r = (color >> 14) * 255 / 127;
        int g = ((color >> 7) & 127) * 255 / 127;
        int b = (color & 127) * 255 / 127;
        Mat ret((int)height * 3 * sy, 
                (int)width * 3 * sx, 
                CV_8UC4, cv::Scalar(b, g, r, 0));
        auto draw = [&ret, width, height] (auto* bitmap, int sx, int sy) {
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
        float tracking = get<float>(this->tracking, time);
        int prevAdvance = 0;
        for (int j = 0; j < s.length(); j++) {
            error = FT_Load_Char(face, s[j], FT_LOAD_RENDER);
            if (error) {
                log(logWARN) << "Can't load character: " << s[j];
                x += size * 2;
            } else {
                x += prevAdvance;
                FT_GlyphSlot slot = face->glyph;
                draw(&slot->bitmap, slot->bitmap_left + x, y - slot->bitmap_top);
                float advance = (slot->advance.x >> 6) - slot->bitmap.width;
                if(advance < 0.5)
                    advance = 0.5;
                prevAdvance = slot->bitmap.width + (1 + tracking) * advance;
            }
        }
        Frame retFrame(ret);
        retFrame.setAnchorAdjustY(size);
        switch(get(this->justification, time)) {
            case 0: // left
                retFrame.setAnchorAdjustX(0);
                break;
            case 1: // center
                retFrame.setAnchorAdjustX(x / 2);
                break;
            case 2: // right
                retFrame.setAnchorAdjustX(x);
                break;
        };
        rendered.insert(i);
        std::string fp = getFramePath(i);
        retFrame.write(fp, 75);
    }
}

int TextRenderable::findKey(int f) const {
    if (keyframes.empty())
        return 0;
    for (int i = keyframes.size() - 1; i >= 0; i--) {
        if (keyframes[i] <= f) 
            return keyframes[i];
    }
    log(logWARN) << "TextRenderable: Couldn't find key frame for " << f;
    return keyframes[0];
}

int TextRenderable::findKeyByTime(float t) const {
    return findKey(getFrameNumber(t));
}
