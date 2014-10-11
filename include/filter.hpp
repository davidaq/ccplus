#pragma once

#include "global.hpp"

namespace CCPlus {
    class FilterLoader;
}

typedef void (*CCPLUS_FILTER_FUNC) (CCPlus::Frame& input, const std::vector<float>& parameters, int width, int height);

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const std::string& name);
    ~Filter();
    
    virtual void apply(CCPlus::Frame& image, const std::vector<float>& parameters, int width, int height);
    
private:
    CCPLUS_FILTER_FUNC func;

    CCPlus::Profiler* profiler;
};

class CCPlus::FilterLoader {
public:
    FilterLoader();
    ~FilterLoader();
};

extern CCPlus::FilterLoader CCPlus__FilterLoader;

#define CCPLUS_FILTER(NAME) \
void _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(CCPlus::Frame& frame, const std::vector<float>& parameters, int width, int height);
#include "filter-list.hpp"
#undef CCPLUS_FILTER

#define CCPLUS_FILTER(NAME) \
void _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(CCPlus::Frame& frame, const std::vector<float>& parameters, int width, int height)

