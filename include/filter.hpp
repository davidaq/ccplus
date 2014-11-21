#pragma once

#include "global.hpp"

namespace CCPlus {
    class FilterLoader;
}

typedef CCPlus::GPUFrame (*CCPLUS_FILTER_FUNC) (CCPlus::GPUFrame input, const std::vector<float>& parameters, int width, int height);

extern std::vector<std::string>* filterOrder;

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const std::string& name);
    ~Filter();
    
    CCPlus::GPUFrame apply(CCPlus::GPUFrame input, const std::vector<float>& parameters, int width, int height);
    
private:
    CCPLUS_FILTER_FUNC func;
};

class CCPlus::FilterLoader {
public:
    FilterLoader();
    ~FilterLoader();
};

extern CCPlus::FilterLoader CCPlus__FilterLoader;

#define CCPLUS_FILTER(NAME) CCPlus::GPUFrame _CCPLUS_FILTER_##NAME##_FILTER_AAPLY( \
        CCPlus::GPUFrame frame, const std::vector<float>& parameters, int width, int height);
#include "filter-list.hpp"
#undef CCPLUS_FILTER

#define CCPLUS_FILTER(NAME) CCPlus::GPUFrame _CCPLUS_FILTER_##NAME##_FILTER_AAPLY( \
        CCPlus::GPUFrame frame, const std::vector<float>& parameters, int width, int height)

