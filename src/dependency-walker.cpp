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
    for(auto& item : fragments) {
        calcItem(item.first, item.second);
    }
}

void DependencyWalker::scan(Composition* comp, Range* parent) {
    for(Layer& layer : comp->layers) {
        Renderable* renderable = layer.getRenderObject();
        Composition* child = dynamic_cast<Composition*>(renderable);
        Range* range = new Range {
            .parent = parent,
            .left = layer.time,
            .right = layer.time + layer.duration,
            .refStart = layer.start,
            .refEnd = layer.start + layer.last,
            .maxDuration = renderable->getDuration()
        };
        ranges.push_back(range);
        if(child) {
            scan(child, range);
        }
        fragments[renderable].push_back(range);
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
                    range.refStart = range.refEnd -
                        (range.refEnd - range.refStart) * (range.right - left) / (range.right - range.left);
                    range.left = left;
                }
                if(range.right > right) {
                    range.refEnd = range.refStart +
                        (range.refEnd - range.refStart) * (right - range.left) / (range.right - range.left);
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
        float d = range.right - range.left;
        range.left += translate;
        range.right = range.left + scale * d;
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
std::string toString(const Range& range) {
    char buff[40];
    sprintf(buff, "[%.3f:%.3f, %.3f:%.3f, %.3f]", range.left, range.right, range.refStart, range.refEnd, range.maxDuration);
    return buff;
}
std::string toString(RangeSet set) {
    std::string ret = "";
    for(const Range& range : set) {
        ret += toString(range);
    }
    return "{" + ret + "}";
}
std::string toString(std::vector<TimePair> set) {
    std::string ret = "";
    for(const TimePair& range : set) {
        char buff[40];
        sprintf(buff, "[%.3f:%.3f]", range.first, range.second);
        ret += buff;
    }
    return "{" + ret + "}";
}

void DependencyWalker::calcItem(Renderable* item, std::vector<Range*> chunks) {
    RangeSet set;
    log(logINFO) << "dependency --" << item->getUri();
    for(const auto & chunk : chunks) {
        set = concat(set, calcChunk(item, chunk));
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
    if(item->firstAppearTime > item->lastAppearTime)
        item->firstAppearTime = item->lastAppearTime;
    simplify(item->usedFragments);
    log(logINFO) << "-- from" << item->firstAppearTime << "to" << item->lastAppearTime
        << "using" << toString(item->usedFragments);
}

RangeSet DependencyWalker::calcChunk(Renderable* item, Range* chunk) {
    RangeSet set;
    Range full;
    full.left = 0;
    full.right = chunk->maxDuration;
    full.refStart = 0;
    full.refEnd = chunk->maxDuration;
    full.maxDuration = 0;
    set.push_back(full);
    log(logINFO) << "full" << toString(full);
    while(chunk) {
        log(logINFO) << "chunk" << toString(*chunk);
        explode(set, *chunk);
        log(logINFO) << "explode" << toString(set);
        set = transform(set, chunk->left - chunk->refStart,
                (chunk->right - chunk->left) / (chunk->refEnd - chunk->refStart));
        log(logINFO) << "transform" << toString(set);
        chunk = chunk->parent;
    }
    return set;
}

