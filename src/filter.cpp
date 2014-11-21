#define CCPLUS_FILTER_SELF
#include "filter.hpp"
#include "gpu-frame.hpp"

#include "profile.hpp"

using namespace CCPlus;

std::map<std::string, CCPLUS_FILTER_FUNC> *filterMap = 0;
std::vector<std::string> *filterOrder = 0;

Filter::Filter(const std::string& name) {
    if(!filterMap || !filterMap->count(name)) {
        log(logWARN) << "Couldn't find filter: " << name;
        func = 0;
    } else {
        func = (*filterMap)[name];
    }
}

Filter::~Filter() {
}

GPUFrame Filter::apply(GPUFrame frame, const std::vector<float>& parameters, int width, int height) {
    if(func) {
        frame = func(frame, parameters, width, height);
    }
    return frame;
}

FilterLoader::FilterLoader() {
    filterMap = new std::map<std::string, CCPLUS_FILTER_FUNC>();
    filterOrder = new std::vector<std::string>();
    #undef CCPLUS_FILTER
    #define CCPLUS_FILTER(NAME) \
    (*filterMap)[#NAME] = _CCPLUS_FILTER_##NAME##_FILTER_AAPLY; \
    (*filterOrder).push_back(#NAME);
    #include "filter-list.hpp"
}

FilterLoader::~FilterLoader() {
    delete filterMap;
    delete filterOrder;
}

FilterLoader CCPlus__FilterLoader;

