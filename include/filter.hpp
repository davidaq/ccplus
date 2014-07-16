#pragma once

#include "Object.hpp"
#include <vector>

namespace CCPlus {
    class Filter;
    class Image;
}

typedef void (*FilterFunction)(const CCPlus::Image * const src, CCPlus::Image* dest, std::vector<float> parameters);

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const char* name);
    ~Filter();
    
    void apply(const CCPlus::Image * const src, CCPlus::Image* dest, std::vector<float> parameters);
    
private:
    
};
