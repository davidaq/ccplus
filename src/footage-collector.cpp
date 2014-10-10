#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"

#include <algorithm>

using namespace CCPlus;

FootageCollector::FootageCollector(const Composition* comp) {
    //const std::vector<Layer> layers = comp->getLayers();
    //for (auto& l : layers) {
    //    if (dynamic_cast<Composition*>(l->getRenderObject())) {
    //        FootageCollector tmp(l->getRenderObject());
    //        mergeDependency(&l, tmp.getDependency());
    //    } else {
    //        FootageDependency dep;
    //        dep.renderable = l->getRenderObject();
    //        // DONE by Layer so 0 <= firstAppear <= comp->getDuration
    //        //dep.firstAppear = std::max(l->getTime(), 0.0f);
    //        //if (dep.firstAppear > comp->getDuration())
    //        //    continue;
    //        dep.firstAppear = l->getTime();
    //        dep.lastAppear = dep.firstAppear() + l->getDuration();
    //        float start = l->getStart();
    //        float end = start + l->getLast();
    //        // DONE by Layer so 0 <= start <= renderable->getDuration()
    //        //while (start >= renderable->getDuration())
    //        //    start -= dep.renderable->getDuration();
    //        //while (start < 0)
    //        //    start += dep.renderable->getDuration();
    //        dep.segments.push_back({start, end});
    //        this->dependencies.push_back(dep);
    //    }
    //}

    ///* Unique */
    //std::vector<FootageDependency> tmpdeps = dependencies;
    //dependencies.clear();
    //std::sort(tmpdeps.begin(), tmpdeps.end(), [] (const FootageDependency& a, const FootageDependency& b) {
    //    return a.renderable < b.renderable;
    //});
    //int cnt = 0;
    //for (int i = 0; i < tmpdeps.size(); i++) {
    //    if (cnt == 0 || dependencies[cnt - 1].renderable != tmpdeps[i].renderable) {
    //        cnt++;
    //        dependencies.push_back(tmpdeps[i]);
    //    } else {

    //        dependencies[cnt - 1].firstAppear = std::min(
    //                dependencies[cnt - 1].firstAppear,
    //                tmpdeps[i].firstAppear);
    //        dependencies[cnt - 1].lastAppear = std::max(
    //                dependencies[cnt - 1].lastAppear,
    //                tmpdeps[i].lastAppear);
    //        dependencies[cnt - 1].segments = mergeSegments(
    //                dependencies[cnt - 1].segments,
    //                tmpdeps[i].segments);
    //    }
    //}

    ///* Sort */
    //dependencies.sort();
}

FootageCollector::~FootageCollector() {
}

void FootageCollector::prepare() {}

void FootageCollector::mergeDependency(Layer* l, const std::vector<FootageDependency>& deps) {
    //// Comp duration
    //float rd = l->getRenderObject()->getDuration();

    //float start = l->getStart();
    //float end = start + l->getLast();
    //float time = l->getTime();
    //float duration = l->getDuration();
    //// Duration + time < upper renderable's duration
    //for (int i = 0; i < deps.size(); i++) {
    //    FootageDependency dp;
    //    dp.renderable = deps[i].renderable;
    //    dp.segments = deps[i].segments;
    //    
    //    float s1 = deps[i].firstAppear;
    //    float e1 = deps[i].lastAppear;
    //    float s2 = deps[i].firstAppear + rd;
    //    float e2 = deps[i].lastAppear + rd;
    //    // BLACK MAGIC
    //    dp.firstAppear = 0x7fffffff;
    //    dp.lastAppear = -0x7fffffff;
    //    if (std::max(s1, start) <= std::min(e1, end)) {
    //        dp.firstAppear = std::min(dp.firstAppear, 
    //                std::max(s1, start));
    //        dp.secondAppear = std::max(dp.lastAppear
    //                std::min(e1, end));
    //    } 
    //    if (std::max(s2, start) <= std::min(e1, end)) {
    //        dp.firstAppear = std::min(dp.firstAppear, 
    //                std::max(s2, start));
    //        dp.secondAppear = std::max(dp.lastAppear
    //                std::min(e2, end));
    //    }

    //    if (dp.firstAppear > dp.lastAppear) continue; 

    //    dp.firstAppear = time + (dp.firstAppear - start) / (end - start) * rd;
    //    dp.lastAppear = time + (dp.lastAppear - start) / (end - start) * rd;
    //    
    //    this->dependencies.push_back(dp);
    //}
}

typedef std::vector<std::pair<float, float> > segs_t;
segs_t FootageCollector::mergeSegments(const segs_t& s1, const segs_t& s2) {
    segs_t ret;
    return ret;  
}
