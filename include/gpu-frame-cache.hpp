#pragma once
#include "global.hpp"

class CCPlus::GPUFrameCache : public Object {
public:
    static GPUFrame alloc(int width, int height);
    static void reuse(GPUFrameImpl*);

protected:
    struct Size {
        int width, height;
        Size(int _w, int _h) {
            width = _w; 
            height = _h;
        }
        int ord() const {
            // Assume texture size are smaller than 10000
            return width * 10000 + height;
        }
        bool operator<(const Size& sz) const {
            return ord() < sz.ord(); 
        }
        bool operator==(const Size& sz) const {
            return ord() == sz.ord();
        }
    };
    // @first -> textureID, @second -> fboID
    static std::map<Size, std::vector<std::pair<GLuint, GLuint>>> cache;
};
