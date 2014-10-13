#pragma once

#include "global.hpp"

namespace CCPlus {
    struct Range {
        Range* parent;
        float left, right, refStart, refEnd, maxDuration;
    };
}

class CCPlus::DependencyWalker : public CCPlus::Object {
public:
    DependencyWalker(CCPlus::Composition& mainComp);
    ~DependencyWalker();
    void walkThrough();
private:
    void scan(CCPlus::Composition* , CCPlus::Range* );
    void cleanup();
    void calcItem(CCPlus::Renderable* item, std::vector<CCPlus::Range*> chunks);
    void simplify(std::vector<std::pair<float,float> >& );
    void calcChunk(CCPlus::Renderable*, CCPlus::Range* chunk);
    std::map<CCPlus::Renderable*, std::vector<CCPlus::Range*> > fragments;
    std::vector<CCPlus::Range*> ranges;
    CCPlus::Composition* mainComp;
};
