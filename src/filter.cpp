#define CCPLUS_FILTER_SELF
#include "filter.hpp"
#include "gpu-frame.hpp"

#include "profile.hpp"

using namespace CCPlus;

std::map<std::string, CCPLUS_FILTER_FUNC> *filterMap = 0;

Filter::Filter(const std::string& name) {
    if(!filterMap || !filterMap->count(name)) {
        this->profiler = 0;
        func = 0;
    } else {
        this->profiler = new Profiler("Filter_" + name);       
        func = (*filterMap)[name];
    }
}

Filter::~Filter() {
    if(profiler)
        delete profiler;
}

GPUFrame Filter::apply(GPUFrame frame, const std::vector<float>& parameters, int width, int height) {
    if(func) {
        profileBegin(Filters);
        if(frame && frame->width > 0 && frame->height > 0)
            frame = func(frame, parameters, width, height);
        profileEnd(Filters);
    }
    return frame;
}

FilterLoader::FilterLoader() {
    filterMap = new std::map<std::string, CCPLUS_FILTER_FUNC>();
    #undef CCPLUS_FILTER
    #define CCPLUS_FILTER(NAME) \
    (*filterMap)[#NAME] = _CCPLUS_FILTER_##NAME##_FILTER_AAPLY;
    #include "filter-list.hpp"
}

FilterLoader::~FilterLoader() {
    delete filterMap;
}

FilterLoader CCPlus__FilterLoader;

