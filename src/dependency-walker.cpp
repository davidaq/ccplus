#include "dependency-walker.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "profile.hpp"

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
    profile(CalcDependency) {
        scan(mainComp, 0);
        for(auto& item : fragments) {
            calcItem(item.first, item.second);
        }
    }
}

void DependencyWalker::scan(Composition* comp, Range* parent, float from, float to) {
    if(to > comp->getDuration())
        to = comp->getDuration();
    if(from < 0)
        from = 0;
    for(Layer& layer : comp->layers) {
        if(layer.time + layer.duration < from || layer.time > to) {
            continue;
        }
        Renderable* renderable = layer.getRenderObject();
        if(!renderable)
            continue;
        Composition* child = dynamic_cast<Composition*>(renderable);
        Range* range = new Range {
            .parent = parent,
            .left = layer.time,
            .right = layer.time + layer.duration,
            .refStart = layer.start,
            .refEnd = layer.start + layer.last,
            .maxDuration = renderable->getDuration(),
        };
        ranges.push_back(range);
        if(child) {
            float cf = layer.mapInnerTime(from);
            float ct = layer.mapInnerTime(to);
            float lf = layer.mapInnerTime(range->left);
            float lt = layer.mapInnerTime(range->right);
            if(lf > cf)
                cf = lf;
            if(lt < ct)
                ct = lt;
            scan(child, range, cf, ct);
        }
        fragments[renderable].push_back(range);
    }
}

static inline void concat(RangeSet& a, const RangeSet& b) {
    if(b.empty())
        return;
    a.reserve(a.size() + b.size());
    a.insert(a.end(), b.begin(), b.end());
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
        concat(set, transform(original, d, 1));
        d += chunk.maxDuration;
    }
    d = 0;
    while(d > chunk.refStart) {
        d -= chunk.maxDuration;
        concat(set, transform(original, d, 1));
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
        if(cright < pair.second)
            cright = pair.second;
    }
    if(cright > cleft) {
        ranges.push_back(TimePair(cleft, cright));
    }
}
std::string toString(const Range& range) {
    char buff[140];
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

void DependencyWalker::generateFragmentSlices(CCPlus::Renderable* item, const RangeSet& set) {
    float left = 0; 
    int idx = 0;

    while (left < mainComp->duration) {
        float right = std::min(left + CCPlus::collectorTimeInterval, mainComp->duration);
        std::vector<std::pair<float, float> > fragments;
        for (const Range& r : set) {
            float newleft = std::max<float>(left, r.left);
            float newright = std::min<float>(right, r.right);
            if (newright < newleft) continue;
            auto innerTime = [&r] (float time) {
                return (time - r.left) / (r.right - r.left) * (r.refEnd - r.refStart) + r.refStart;
            };
            fragments.push_back(std::make_pair(innerTime(newleft), innerTime(newright)));
        } 
        if (fragments.size()) {
            simplify(fragments);
            item->usedFragmentSlices[idx] = fragments;
        }
        idx += 1;
        left += CCPlus::collectorTimeInterval;
    }
}

void DependencyWalker::calcItem(Renderable* item, std::vector<Range*> chunks) {
    RangeSet set;
    log(logINFO) << "dependency --" << item->getUri();
    for(const auto & chunk : chunks) {
        concat(set, calcChunk(item, chunk));
    }
    crop(set, 0, mainComp->duration);

    // Calculate firstAppearTime and lastAppearTime
    item->firstAppearTime = 9999999;
    item->lastAppearTime = 0;
    for(const Range& r : set) {
        if(r.left < item->firstAppearTime)
            item->firstAppearTime = r.left;
        if(r.right > item->lastAppearTime)
            item->lastAppearTime = r.right;
    }

    generateFragmentSlices(item, set);

    log(logINFO) << "-- from" << item->firstAppearTime << "to" << item->lastAppearTime;
        //<< "using" << toString(item->usedFragments);
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
    //L() << "full" << toString(full);
    while(!set.empty() && chunk) {
        if(chunk->maxDuration <= 0)
            return RangeSet();
        //L() << "chunk" << toString(*chunk);
        explode(set, *chunk);
        //L() << "explode" << toString(set);
        set = transform(set, chunk->left - chunk->refStart,
                (chunk->right - chunk->left) / (chunk->refEnd - chunk->refStart));
        //L() << "transform" << toString(set);
        chunk = chunk->parent;
    }
    //L() << "++++++++++++ portion ------" << toString(set) << item->getUri();
    return set;
}

