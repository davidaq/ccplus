#pragma once

#include "renderable.hpp"
#include <map>
#include <string>

namespace CCPlus {
    class TextRenderable;
}

class CCPlus::TextRenderable : public CCPlus::Renderable {
public:
    TextRenderable(CCPlus::Context* context, const std::string& uri);

    void render(float start, float duration);

    Frame getFrame(float time) const;
    Frame getFrameByNumber(int frame) const;

    float getDuration() const;
    int getWidth() const;
    int getHeight() const;

    bool still(float t1, float t2);

    void clear();

    const std::string& getName() const;
    
    std::map<float, std::wstring> text;
    std::map<float, std::wstring> font;
    std::map<float, int> size;
    std::map<float, float > scale_x;
    std::map<float, float > scale_y;
    std::map<float, float> tracking;
    std::map<float, bool> bold;
    std::map<float, bool> italic;

protected:
    template<typename T> 
    T get(const std::map<float, T>& m, float t) const {
        float ret;
        // TODO: binary search
        for (auto& kv : m) {
            if (kv.first <= t) 
                ret = kv.first;
            if (kv.first > t)
                break;
        }
        return m.at(ret);
    }

    int findKey(int f) const;
    int findKeyByTime(float f) const;

    std::vector<int> keyframes;
    std::set<int> rendered;

    std::string uri;
};
