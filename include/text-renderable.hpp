#pragma once

#include "renderable.hpp"
#include <boost/property_tree/ptree.hpp> 

namespace CCPlus {
    class TextRenderable;
}

class CCPlus::TextRenderable : public CCPlus::Renderable {
public:
    TextRenderable(const boost::property_tree::ptree& properties);
    ~TextRenderable() {this->release();};

    void prepare();

    float getDuration();
    CCPlus::GPUFrame getGPUFrame(float time);
    void release();

    int getWidth() const;
    int getHeight() const;

protected:
    std::map<int, std::wstring> text;
    std::map<int, std::string> font;
    std::map<int, int> size;
    std::map<int, float > scale_x;
    std::map<int, float > scale_y;
    std::map<int, float> tracking;
    std::map<int, bool> bold;
    std::map<int, bool> italic;
    std::map<int, int> color;
    std::map<int, int> justification;

    template<typename T> 
    T get(const std::map<int, T>& m, int t) const {
        int ret;
        for (auto& kv : m) {
            if (kv.first <= t) 
                ret = kv.first;
            if (kv.first > t)
                break;
        }
        if (!m.count(ret)) {
            for (auto& kv : m)
                return kv.second;
        }
        return m.at(ret);
    }

    void prepareFrame(int time);
    int findKeyTime(float time);
    std::vector<int> keyframes;
    std::map<int, Frame> framesCache;
    std::map<int, GPUFrame> gpuFramesCache;
};
