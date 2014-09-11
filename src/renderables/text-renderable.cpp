#include "text-renderable.hpp"
#include "frame.hpp"
#include "logger.hpp"
#include <algorithm>

extern "C" 
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

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

    // Init FreeType
    FT_Library library;
    FT_Face face;
    int error;
    error = FT_Init_FreeType(&library);
    if ( error  ) {
        log(logFATAL) << "Can't initialize FreeType";
    }
    error = FT_New_Face( library,
            "/Library/Fonts/华文黑体.ttf",
            0,
            &face);
    if ( error  ) {
        log(logFATAL) << "Can't load font...";
    }

    for (int i = startFrame; i <= endFrame; i++) {
        // Come on FreeType!
        float time = getFrameTime(i);
        int size = get(this->size, time);
        float sx = get<float>(this->scale_x, time);
        float sy = get<float>(this->scale_y, time);
        if (FT_Set_Pixel_Sizes(face, (int)size * sx, (int)size * sy)) {
            log(logFATAL) << "Can't set text size";
        }
        Mat ret((int)height * 2 * sy, 
                (int)width * 2 * sx, 
                CV_8UC4, {255, 255, 255, 0});
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
        for (int j = 0; j < s.length(); j++) {
            error = FT_Load_Char(face, s[j], FT_LOAD_RENDER);
            if (error) {
                log(logFATAL) << "Can't load character: " << s[j];
                return;
            }

            FT_GlyphSlot slot = face->glyph;
            if (i == 180) {
                L() << ret.rows << " " << ret.cols;
                L() << slot->bitmap_left + x << " " << 
                    y - slot->bitmap_top;
            }

            draw(&slot->bitmap, slot->bitmap_left + x, y - slot->bitmap_top);

            int advance = (slot->advance.x >> 6) - slot->bitmap.width;
            advance = slot->bitmap.width + advance + tracking * advance;
            x += advance;
        }
        Frame retFrame(ret);
        rendered.insert(i);
        std::string fp = getFramePath(i);
        retFrame.write(fp, 75);
    }

    FT_Done_FreeType(library);
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
