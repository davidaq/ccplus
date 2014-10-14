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

RangeSet concat(const RangeSet& a, const RangeSet& b) {
    RangeSet ret;
    ret.reserve(a.size() + b.size());
    ret.insert(ret.end(), a.begin(), a.end());
    ret.insert(ret.end(), b.begin(), b.end());
    return ret;
}
RangeSet crop(const RangeSet& a, float left, float right) {
    RangeSet ret;
    if(left < right) {
        ret.reserve(a.size() * 0.7);
        for(Range range : a) {
            if(range.right > left && range.left < right) {
                if(range.left < left) {
                    range.refStart = range.refEnd - (range.refEnd - range.refStart) * (range.right - left) / (range.right - range.left);
                    range.left = left;
                }
                if(range.right > right) {
                    range.refEnd = range.refStart + (range.refEnd - range.refStart) * (right - range.left) / (range.right - range.left);
                    range.right = right;
                }
                ret.push_back(range);
            }
        }
    }
    return ret;
}
RangeSet transform(const RangeSet& a, float translate, float scale) {
    RangeSet ret;
    ret.reserve(a.size());
    for(Range range : a) {
        range.left += translate;
        range.right = range.left + scale * (range.right + translate - range.left);
        ret.push_back(range);
    }
    return ret;
}
void explode(RangeSet& set, Range chunk) {
    float d = chunk.maxDuration;
    RangeSet original = set;
    while(d < chunk.refEnd) {
        set = concat(set, transform(original, d, 1));
        d += chunk.maxDuration;
    }
    set = crop(set, chunk.refStart, chunk.refEnd);
}
typedef std::pair<float,float> TimePair;
void simplify(std::vector<TimePair>& ranges) {
    std::sort(ranges.begin(), ranges.end(), 
            [](TimePair const & a, TimePair const & b) {
        return a.first < b.first;
    });
    std::vector<TimePair> sorted = ranges;
    ranges.clear();
    float cleft = 0;
    float cright = 0;
    for(const TimePair& pair : sorted) {
        if(pair.first > cright) {
            if(cright > cleft) {
                ranges.push_back(TimePair(cleft, cright));
            }
            cleft = pair.first;
        }
        cright = pair.second;
    }
    if(cright > cleft) {
        ranges.push_back(TimePair(cleft, cright));
    }
}

void DependencyWalker::calcItem(Renderable* item, std::vector<Range*> chunks) {
    RangeSet set;
    for(std::vector<Range*>::iterator chunk = chunks.begin(); chunk != chunks.end(); chunk++) {
        set = concat(set, calcChunk(item, *chunk));
    }
    item->firstAppearTime = 9999999;
    item->lastAppearTime = 0;
    for(const Range& r : set) {
        if(r.left < item->firstAppearTime)
            item->firstAppearTime = r.left;
        if(r.right > item->lastAppearTime)
            item->lastAppearTime = r.right;
        item->usedFragments.push_back(std::pair<float,float>(r.refStart, r.refEnd));
    }
    simplify(item->usedFragments);
}

RangeSet DependencyWalker::calcChunk(Renderable* item, Range* chunk) {
    RangeSet set;
    Range full;
    full.left = 0;
    full.right = chunk->maxDuration;
    full.refStart = 0;
    full.refEnd = chunk->maxDuration;
    set.push_back(full);
    while(chunk) {
        explode(set, *chunk);
        set = transform(set, chunk->left - chunk->refStart, (chunk->right - chunk->left) / (chunk->refStart - chunk->refEnd));
        chunk = chunk->parent;
    }
    return set;
    //if(range.left < item->firstAppearTime)
    //    item->firstAppearTime = range.left;
    //if(range.right > item->lastAppearTime)
    //    item->lastAppearTime = range.right;
    //item->usedFragments.push_back(
    //    std::pair<float,float>(range.refStart, range.refEnd - range.refStart)
    //);
}

