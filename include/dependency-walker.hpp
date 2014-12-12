#pragma once

#include "global.hpp"

namespace CCPlus {
    struct Range {
        Range* parent;
        float left, right, refStart, refEnd, maxDuration;
    };
    typedef std::vector<Range> RangeSet;
}

class CCPlus::DependencyWalker : public CCPlus::Object {
public:
    DependencyWalker(CCPlus::Composition& mainComp);
    ~DependencyWalker();
    void walkThrough();
private:
    void scan(CCPlus::Composition*, CCPlus::Range*, float from=0, float to=99999.0f);
    void cleanup();
    void calcItem(CCPlus::Renderable* item, std::vector<CCPlus::Range*> chunks);
    CCPlus::RangeSet calcChunk(CCPlus::Renderable*, CCPlus::Range* chunk);
    std::map<CCPlus::Renderable*, std::vector<CCPlus::Range*> > fragments;
    std::vector<CCPlus::Range*> ranges;
    CCPlus::Composition* mainComp;
};
