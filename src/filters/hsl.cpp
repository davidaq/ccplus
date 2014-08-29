#include "filter.hpp"
#include "utils.hpp"
#include "logger.hpp"

using namespace cv;
using namespace CCPlus;

void bgr2hsl(Vec4b& pixel);
void hsl2bgr(Vec4b& pixel);

CCPLUS_FILTER(hsl) {
    if (parameters.size() < 3) return;

    float hue = parameters[0]; // 0 ~ 180
    float sat = parameters[1]; // 0 ~ 2.0
    float lit = parameters[2]; // 0 ~ 2.0
    
    Mat &img = frame.getImage();

    Vec4b* ptr = img.ptr<Vec4b>(0);
    for(int i = 0, c = img.total(); i < c; i++) {
        Vec4b& pixel = *(ptr++);
        bgr2hsl(pixel);
        int p = pixel[0];
        p += hue;
        while(p >= 180)
            p -= 180;
        while(p < 0)
            p += 180;
        pixel[0] = p;
        
        // L
        float now_lit = pixel[1];
        float diff = (lit - 1.0) * (lit > 1.0 ? (255 - now_lit) : now_lit);
        float ret = between<float>(now_lit + diff, 0.0, 255.0);
        pixel[1] = (unsigned char) ret;

        // S
        float now_sat = pixel[2];
        diff = (sat - 1.0) * (sat > 1.0 ? (255 - now_sat) : now_sat);
        ret = between<float>(now_sat + diff, 0.0, 255.0);
        pixel[2] = (unsigned char) ret;

        hsl2bgr(pixel);
    }
}

void bgr2hsl(Vec4b& pixel) {
    uint8_t h, s, l;
    uint8_t r = pixel[2], g = pixel[1], b = pixel[0];
    int max = pixel[0];
    int min = pixel[0];
    for(int i = 1; i < 3; i++) {
        if(pixel[i] > max)
            max = pixel[i];
        else if(pixel[i] < min)
            min = pixel[i];
    }
    // hue
    if(max == min) {
        h = 0;
    } else if(max == r) {
        if(g >= b)
            h = (g - b) * 30 / (max - min);
        else
            h = (g - b) * 30 / (max - min) + 180;
    } else if(max == g) {
        h = ((b - r) * 30 / (max - min)) + 60;
    } else {
        h = ((r - g) * 30 / (max - min)) + 120;
    }
    l = (max + min) / 2;
    if(l == 0 || max == min)
        s = 0;
    else if(l <= 127) {
        s = (255 * (max - min)) / (max + min);
    } else {
        s = (255 * (max - min)) / (510 - max - min);
    }

    pixel[0] = h;
    pixel[1] = l;
    pixel[2] = s;
}

void hsl2bgr(Vec4b& pixel) {
    uint8_t s = pixel[2], l = pixel[1], h = pixel[0];
    if(s == 0)
        pixel[0] = pixel[1] = pixel[2] = l;
    else {
        int bgr[3] = {h - 60, h, h + 60};
        int q = (l < 127) ? l * (255 + s) : (l + s) * 255 - l * s;
        int p = 510 * l - q;
        for(int i = 0; i < 3; i++) {
            int &c = bgr[i];
            while(c < 0)
                c += 255;
            while(c > 255)
                c -= 255;
            if(c < 30) {
                c = p + (q - p) * 6 * c / 180;
            } else if(c < 90) {
                c = q;
            } else if(c < 120) {
                c = p + (q - p) * 6 * (120 - c) / 180;
            } else {
                c = p;
            }
            pixel[i] = c / 255;
        }
    }
}
