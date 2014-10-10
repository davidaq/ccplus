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

private:
    typedef std::vector<std::pair<float, float> > segs_t;
    struct FootageDependency {
        CCPlus::Renderable* renderable;
        float firstAppear, lastAppear; 
        segs_t segments;
    };

    void mergeDependency(Layer* l, const std::vector<FootageDependency>& deps);
    segs_t mergeSegments(const segs_t&, const segs_t&);
    float* tl_pointers;
    std::vector<FootageDependency> dependencies;
};
