#pragma once

#include "image.hpp"
#include <vector>
#include <string>
#include <map>

namespace CCPlus {
    class FilterLoader;
    class Filter;
    class Image;
}

typedef cv::Mat (*CCPLUS_FILTER_FUNC) (cv::Mat input, const std::vector<float>& parameters, int width, int height);

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const std::string& name);
    
    virtual void apply(CCPlus::Image& image, const std::vector<float>& parameters, int width, int height);
    
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
cv::Mat _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(cv::Mat input, const std::vector<float>& parameters, int width, int height);
#include "filter-list.hpp"
#undef CCPLUS_FILTER

#define CCPLUS_FILTER(NAME) \
cv::Mat _CCPLUS_FILTER_##NAME##_FILTER_AAPLY(cv::Mat input, const std::vector<float>& parameters, int width, int height)

#include "global.hpp"
