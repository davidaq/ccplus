#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "context.hpp"
#include "dependency-walker.hpp"
#include "parallel-executor.hpp"

#include <algorithm>

using namespace CCPlus;

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
}

FootageCollector::~FootageCollector() {
    stop();
}

void FootageCollector::prepare() {
    ParallelExecutor::runInNewThread([this] () {
        this->doPrepare();
    });
}

void FootageCollector::doPrepare() {
    DependencyWalker dep(*main);
    dep.walkThrough();

    finishedTime = 0;
    int idx = 0;

    std::map<std::string, Renderable*>& renderables = Context::getContext()->renderables;
    while (finishedTime < main->duration && continueRunning) {
        if (finishedTime - renderTime > windowDuration) {
            //L() << "I'm waiting" << finishedTime << renderTime;
            usleep(10000);
            continue;
        }
        for (auto& kv : renderables) {
            Renderable* renderable = kv.second;
            if (renderable->usedFragmentSlices.count(idx)) {
                std::vector<std::pair<float, float> >* fragments = &renderable->usedFragmentSlices[idx];
                for (auto& i : *fragments) {
                    //L() << "Preparing: " << renderable->getUri() << i.first << "~" << i.second;
                    //renderable->preparePart(std::max(0.0, i.first - 0.3), i.second - i.first + 0.5);
                    renderable->preparePart(i.first, i.second - i.first);
                }
            }

            if (renderTime > renderable->lastAppearTime) {
                renderable->release();
            }
        }

        //executor.waitForAll();
        finishedTime += CCPlus::collectorTimeInterval;
        idx++;
        log(logINFO) << "Already prepared: " << finishedTime << "seconds.";
    }
}

void FootageCollector::stop() {
    continueRunning = false;
}

void FootageCollector::clean(float time) {
    renderTime = time;
}

float FootageCollector::finished() {
    return finishedTime;
}
