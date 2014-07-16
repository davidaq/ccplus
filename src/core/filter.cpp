#include "filter.hpp"

using namespace CCPlus;

std::map<std::string, FilterFunction> Filter::filterFunctions;
    
Filter::Filter(const std::string& name) {
    if(filterFunctions.count(name)) {
        filterFunction = filterFunctions[name];
    } else {
        filterFunction = 0;
    }
    printf("get filter %s\n", name.c_str());
}

Filter::Filter(const std::string& name, FilterFunction logic) {
    printf("define filter %s\n", name.c_str());
    //filterFunctions.insert(std::make_pair(name, filterFunction = logic));
}

void Filter::apply(const CCPlus::Image * const src, CCPlus::Image* dest, const std::vector<float>& parameters) {
    if(filterFunction != 0) {
        filterFunction(src, dest, parameters);
    }
}
