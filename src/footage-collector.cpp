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

void FootageCollector::prepare() {
    DependencyWalker dep(*main);
    dep.walkThrough();

    auto& renderables = Context::getContext()->renderables;
    sortedList = new Renderable*[renderables.size() + 2];
    sortedListPtr = 0;
    for(auto ite : renderables) {
        sortedList[sortedListPtr++] = ite.second;
        L() << ite.first << ':' << ite.second->firstAppearTime << '-' << ite.second->lastAppearTime;
    }
    std::sort(sortedList, sortedList + sortedListPtr, [](Renderable* const & a, Renderable* const & b) {
        return a->firstAppearTime < b->firstAppearTime;
    });
    ParallelExecutor::runInNewThread([&]() {
        bool goon = true;
        while(goon) {
            sync.lock();
            if(sortedListPtr > 0) {
                Renderable* pitem = sortedList[--sortedListPtr];
                //finishedTime[0] = pitem->firstAppearTime - 1;
                sync.unlock();
                L() << "prepare";
                pitem->prepare();
                L() << "prepared" << pitem->lastAppearTime;
                sync.lock();
                finishedTime[0] = pitem->lastAppearTime;
            } else {
                finishedTime[0] = main->duration + 1;
                L() << "finished " << finishedTime[0];
                goon = false;
            }
            sync.unlock();
            signal.notify();
        }
    });

    ParallelExecutor::runInNewThread([&]() {
        bool goon = true;
        while(goon) {
            sync.lock();
            if(sortedListPtr > 0) {
                Renderable* pitem = sortedList[--sortedListPtr];
                //finishedTime[1] = pitem->firstAppearTime - 1;
                sync.unlock();
                L() << "prepare";
                pitem->prepare();
                L() << "prepared" << pitem->lastAppearTime;
                sync.lock();
                finishedTime[1] = pitem->lastAppearTime;
            } else {
                finishedTime[1] = main->duration + 1;
                L() << "finished " << finishedTime[1];
                goon = false;
            }
            sync.unlock();
            signal.notify();
        }
    });
}

float FootageCollector::finished() {
    sync.lock();
    float ret = std::min(finishedTime[0], finishedTime[1]);
    sync.unlock();
    return ret;
}

