#define CCPLUS_FILTER_SELF
#include "filter.hpp"

using namespace CCPlus;

std::map<std::string, CCPLUS_FILTER_FUNC> *filterMap = 0;

Filter::Filter(const std::string& name) {
    if(!filterMap || !filterMap->count(name))
        func = 0;
    func = (*filterMap)[name];
}

void Filter::apply(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters) {
    if(func) {
        func(src, dest, parameters);
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
