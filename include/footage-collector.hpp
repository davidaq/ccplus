#pragma once

#include "global.hpp"

/*
 * Calculate dependency 
 * Use multi-thread to prepare footages
 * Return finished time point
 */
class CCPlus::FootageCollector : public CCPlus::Object {
public:
    FootageCollector(const CCPlus::Composition* main);
    ~FootageCollector();

    void prepare();
    // Return the time point before that all preparation was done
    float finished();

    struct FootageDependency {
        CCPlus::Renderable* renderable;
        float firstAppear, lastAppear; 
        std::vector<std::pair<float, float> > segments;
    };

    float* tl_pointers;
    std::vector<CCPlus::Renderable*> deps;
};
