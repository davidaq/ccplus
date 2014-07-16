#pragma once

#include "Object.hpp"
#include <vector>
#include <string>
#include <map>

namespace CCPlus {
    class Filter;
    class Image;
}

typedef void (*FilterFunction)(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters);

class CCPlus::Filter : public CCPlus::Object {
public:
    Filter(const std::string& name);
    Filter(const std::string& name, FilterFunction logic);
    
    void apply(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters);
    
private:
    FilterFunction filterFunction;
    
    static std::map<std::string, FilterFunction> filterFunctions;
};

#define CCPLUS_FILTER(NAME) \
void _CCPLUS_FILTER_##NAME##_IMPLEMENTATION_FUNCTION (const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters); \
CCPlus::Filter _CCPLUS_FILTER_##NAME##_IMPLEMENTATION_OBJ = CCPlus::Filter(#NAME, _CCPLUS_FILTER_##NAME##_IMPLEMENTATION_FUNCTION); \
void _CCPLUS_FILTER_##NAME##_IMPLEMENTATION_FUNCTION (const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters)

#include "global.hpp"
