#include "footage-collector.hpp"
#include "layer.hpp"
#include "object.hpp"
#include "composition.hpp"

#include <algorithm>

using namespace CCPlus;

FootageCollector::FootageCollector(const Composition* comp) {
    const std::vector<Layer> layers = comp->getLayers();
    for (auto& l : layers) {
        if (dynamic_cast<Composition*>(l->getRenderObject())) {
            FootageCollector tmp(l->getRenderObject());
            mergeDependency(l, tmp.getDependency());
        } else {
            FootageDependency dep;
            dep.renderable = l->getRenderObject();
            dep.time = std::max(l->getTime(), 0.0f);
            if (dep.time > comp->getDuration())
                continue;
            dep.start = l->getStart();
            while (dep.start >= dep.renderable->getDuration())
                dep.start -= dep.renderable->getDuration();
            dep.end = dep.start + l->getLast();
            this->dependencies.push_back(dep);
        }
    }

    /* Unique */
    std::vector<FootageDependency> tmpdeps = dependencies;
    dependencies.clear();
    std::sort(tmpdeps.begin(), tmpdeps.end(), [] (const FootageDependency& a, const FootageDependency& b) {
        return a.renderable < b.renderable;
    });
    int cnt = 0;
    for (int i = 0; i < tmpdeps.size(); i++) {
        if (cnt == 0 || dependencies[cnt - 1].renderable != tmpdeps[i].renderable) {
            cnt++;
            dependencies.push_back(tmpdeps[i]);
        } else {
            dependencies[cnt - 1].time = std::min(
                    dependencies[cnt - 1].time,
                    tmpdeps[i].time);
            dependencies[cnt - 1].start = std::min(
                    dependencies[cnt - 1].start,
                    tmpdeps[i].start);
            dependencies[cnt - 1].end = std::max(
                    dependencies[cnt - 1].end,
                    tmpdeps[i].end);
        }
    }

    /* Sort */
    dependencies.sort();
}

FootageCollector::~FootageCollector() {
}

void FootageCollector::prepare() {}

const std::vector<FootageDependency>& FootageCollector::getDependency() const {
    return dependencies;
}

void FootageCollector::mergeDependency(Layer* l, const std::vector<FootageCollector>& deps) {
    // Comp duration
    float rd = l->getRenderObject()->getDuration();

    float start = l->getStart();
    while (start >= rd) start -= rd;
    float end = start + l->getLast();
    float time = l->getTime();
    float duration = l->getDuration();
    time = std::max(0.0f, time);
    if (time > rd) return; // Not showup
    for (int i = 0; i < deps.size(); i++) {
        FootageDependency dp;
        dp.renderable = deps[i].renderable;
        dp.start = deps[i].start;
        dp.end = deps[i].end;
        float tmp = deps[i].time;
        float diff = -1;
        if (tmp >= start && tmp <= end)
            diff = tmp - start;
        else if (tmp + rd <= end)
            diff = tmp + rd - tmp;
        if (diff == -1)
            continue;
        
        dp.time = time + diff / (end - start) * duration;
        
        this->dependencies.push_back(dp);
    }
}
