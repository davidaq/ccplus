#include "dependency-walker.hpp"
#include "layer.hpp"
#include "composition.hpp"

using namespace CCPlus;

DependencyWalker::DependencyWalker(Composition& mainComp) {
    this->mainComp = &mainComp;
}

DependencyWalker::~DependencyWalker() {
    cleanup();
}

void DependencyWalker::cleanup() {
    for(std::vector<Range*>::iterator item = ranges.begin(); item != ranges.end(); item++) {
        delete *item;
    }
    ranges.clear();
    fragments.clear();
}

void DependencyWalker::walkThrough() {
    cleanup();
    scan(mainComp, 0);
    for(std::map<CCPlus::Renderable*, std::vector<CCPlus::Range*> >::iterator item = fragments.begin();
            item != fragments.end(); item++) {
        calcItem(item->first, item->second);
    }
}

void DependencyWalker::scan(Composition* comp, Range* parent) {
    for(std::vector<Layer>::iterator layer = comp->layers.begin(); layer != comp->layers.end(); layer++) {
        Renderable* renderable = layer->getRenderObject();
        Composition* child = dynamic_cast<Composition*>(renderable);
        Range* range = new Range {
            .parent = parent,
            .left = layer->time,
            .right = layer->time + layer->duration,
            .refStart = layer->start,
            .refEnd = layer->start + layer->last,
            .maxDuration = renderable->getDuration()
        };
        ranges.push_back(range);
        if(child) {
            scan(child, range);
        } else {
            fragments[renderable].push_back(range);
        }
    }
}

void DependencyWalker::calcItem(Renderable* item, std::vector<Range*> chunks) {
    item->firstAppearTime = 9999999;
    item->lastAppearTime = 0;
    for(std::vector<Range*>::iterator chunk = chunks.begin(); chunk != chunks.end(); chunk++) {
        calcChunk(item, *chunk);
    }
    simplify(item->usedFragments);
}

typedef std::vector<Range> RangeSet;
RangeSet unionset(const RangeSet& a, const RangeSet& b) {
}
void intersect(RangeSet& a, float left, float right) {
}
RangeSet translate(const RangeSet& a, float d) {
    for(RangeSet::const_iterator i = a.cbegin(); i != a.cend(); i++) {
    }
}
void scale(RangeSet& a, float f) {
}

void DependencyWalker::calcChunk(Renderable* item, Range* chunk) {
    while(chunk) {
        float rEnd = chunk->refEnd;
        float rStart = chunk->refStart;
        
        chunk = chunk->parent;
    }
    //if(range.left < item->firstAppearTime)
    //    item->firstAppearTime = range.left;
    //if(range.right > item->lastAppearTime)
    //    item->lastAppearTime = range.right;
    //item->usedFragments.push_back(
    //    std::pair<float,float>(range.refStart, range.refEnd - range.refStart)
    //);
}

void DependencyWalker::simplify(std::vector<std::pair<float,float> >& ) {
}

