#pragma once

#include "global.hpp"

/*
 * Calculate dependency 
 * Use multi-thread to prepare footages
 * Return finished time point
 */
class FootageCollector : public Object {
public:
    FootageCollector(const Composition* main);
    ~FootageCollector();

    void prepare();
    // Return the time point before that all preparation was done
    float finished();

    float* tl_pointers;
    std::vector<FootageDependency> dependencies;

private:
    typedef std::vector<std::pair<float, float> > segs_t;
    struct FootageDependency {
        Renderable* renderable;
        float firstAppear, lastAppear; 
        segs_t segments;
    };

    void mergeDependency(Layer* l, const std::vector<FootageDependency>& deps);
    segs_t mergeSegments(const segs_t&, const segs_t&);
}

