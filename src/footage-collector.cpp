#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "context.hpp"
#include "dependency-walker.hpp"
#include "parallel-executor.hpp"
#include "ccplus.hpp"

#include <algorithm>

using namespace CCPlus;

pthread_t prepareThread = 0;

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
}

FootageCollector::~FootageCollector() {
    stop();
    if (prepareThread) {
        pthread_join(prepareThread, NULL);
        prepareThread = 0;
    }
}

void FootageCollector::prepare(float startTime) {
    prepareThread = ParallelExecutor::runInNewThread([&] () {
        doPrepare(startTime);
    });
}

void FootageCollector::doPrepare(float startTime) {
    DependencyWalker dep(*main);
    dep.walkThrough();

    finishedTime = 0;
    int idx = 0;

    std::map<std::string, Renderable*>& renderables = Context::getContext()->renderables;
    float windowDuration = collectorWindowSize * collectorTimeInterval * (renderMode == PREVIEW_MODE ? 2 : 1);
    ParallelExecutor executor(collectorThreadsNumber);
    while (finishedTime < main->duration && continueRunning) {
        if (finishedTime - renderTime > windowDuration) {
            //L() << "I'm waiting" << finishedTime << renderTime;
            usleep(10000);
            continue;
        }
        for (auto& kv : renderables) {
            executor.execute([&, kv]() {
                Renderable* renderable = kv.second;
                if (renderable->usedFragmentSlices.count(idx)) {
                    std::vector<std::pair<float, float> >* fragments = &renderable->usedFragmentSlices[idx];
                    for (auto& i : *fragments) {
                        renderable->preparePart(i.first, i.second - i.first);
                    }
                }
                if (renderTime > renderable->lastAppearTime) {
                    renderable->release();
                }
            });
        }

        executor.waitForAll(false);
        idx++;
        finishedTime = idx * collectorTimeInterval;
        signal.notify();
        log(logINFO) << "Already prepared: " << finishedTime << "seconds.";

        int releaseIdx = idx - collectorWindowSize - 2;
        if(releaseIdx >= 0) {
            for (auto& kv : renderables) {
                Renderable* renderable = kv.second;
                if (renderable->usedFragmentSlices.count(releaseIdx)) {
                    std::vector<std::pair<float, float> >* fragments = &renderable->usedFragmentSlices[releaseIdx];
                    for (auto& i : *fragments) {
                        renderable->releasePart(i.first, i.second - i.first);
                    }
                }
            }
        }
    }
    finishedTime = main->duration + 1000.0; // Make sure other thread know its done
    signal.notifyAll();
}

void FootageCollector::stop() {
    continueRunning = false;
}

void FootageCollector::clean(float time) {
    renderTime = time;
}

float FootageCollector::finished() {
    return finishedTime - 0.1;
}

