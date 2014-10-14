#pragma once

#include "renderable.hpp"
#include <map>
#include <string>

namespace CCPlus {
    class TextRenderable;
}

static inline int FI(float v) {
    return (int)(v * 1000);
}

class CCPlus::TextRenderable : public CCPlus::Renderable {
public:
    void prepare();

    float getDuration();
    void updateGPUFrame(GPUFrame& frame, float time);
    void release();

    int getWidth() const;
    int getHeight() const;
    
    std::map<int, std::wstring> text;
    std::map<int, std::wstring> font;
    std::map<int, int> size;
    std::map<int, float > scale_x;
    std::map<int, float > scale_y;
    std::map<int, float> tracking;
    std::map<int, bool> bold;
    std::map<int, bool> italic;
    std::map<int, int> color;
    std::map<int, int> justification;

protected:
    template<typename T> 
    T get(const std::map<int, T>& m, float _t) const {
        int t = FI(_t);
        T ret;
        // TODO: binary search
        for (auto& kv : m) {
            if (kv.first <= t) 
                ret = kv.second;
            if (kv.first > t)
                break;
        }
        return ret;
    }

    void prepareFrame(float time);
    float findKeyTime(float time);
    std::vector<float> keyframes;
    std::map<int, Frame> framesCache;
    std::string uri;
};
