#pragma once

#include "Object.hpp"
#include <vector>
#include <string>
#include <map>

namespace CCPlus {
    class FilterLoader;
    class Filter;
    class Image;
}

typedef void (*CCPLUS_FILTER_FUNC) (const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters);

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const std::string& name);
    
    virtual void apply(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters);
    
private:
    CCPLUS_FILTER_FUNC func;
};

class CCPlus::FilterLoader {
public:
    FilterLoader();
    ~FilterLoader();
};

extern CCPlus::FilterLoader CCPlus__FilterLoader;

#define CCPLUS_FILTER(NAME) \
void _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters);
#include "filter-list.hpp"
#undef CCPLUS_FILTER

#define CCPLUS_FILTER(NAME) \
void _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters)

#include "global.hpp"