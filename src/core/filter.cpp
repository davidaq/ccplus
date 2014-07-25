#define CCPLUS_FILTER_SELF
#include "filter.hpp"
#include "frame.hpp"

using namespace CCPlus;

std::map<std::string, CCPLUS_FILTER_FUNC> *filterMap = 0;

Filter::Filter(const std::string& name) {
    if(!filterMap || !filterMap->count(name))
        func = 0;
    else
        func = (*filterMap)[name];
}

void Filter::apply(Frame& frame, const std::vector<float>& parameters, int width, int height) {
    if(func) {
        func(frame, parameters, width, height);
    }
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
