#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "context.hpp"
#include "dependency-walker.hpp"
#include "parallel-executor.hpp"

#include <algorithm>

using namespace CCPlus;

class CCPlus::CollectorThread : public Object {
public:
    CollectorThread(int index, FootageCollector* collector) :c(*collector) {
        this->index = index;
    }
    void start() {
        ParallelExecutor::runInNewThread([&]() {
            bool goon = true;
            while(goon) {
                c.sync.lock();
                if(c.sortedListPtr > 0) {
                    Renderable* pitem = c.sortedList[--c.sortedListPtr];
                    c.finishedTime[index] = pitem->firstAppearTime - 1;
                    c.sync.unlock();
                    log(logINFO) << "prepare" << pitem->getUri();
                    pitem->prepare();
                    c.sync.lock();
                    c.finishedTime[index] = pitem->lastAppearTime;
                } else {
                    c.finishedTime[index] = c.main->duration + 1;
                    goon = false;
                }
                c.sync.unlock();
                c.signal.notify();
            }
        });
    }
private:
    int index;
    FootageCollector& c;
};

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
    for(int i = 0; i < COLLECTOR_THREAD; i++) {
        finishedTime[i] = 0;
        threads[i] = new CollectorThread(i, this);
    }
}

FootageCollector::~FootageCollector() {
    for(int i = 0; i < COLLECTOR_THREAD; i++) {
        delete threads[i];
    }
    if(sortedList)
        delete[] sortedList;
}

void FootageCollector::prepare() {
    DependencyWalker dep(*main);
    dep.walkThrough();

    auto& renderables = Context::getContext()->renderables;
    sortedList = new Renderable*[renderables.size() + 2];
    sortedListPtr = 0;
    for(auto ite : renderables) {
        if(ite.second != main)
            sortedList[sortedListPtr++] = ite.second;
    }
    std::sort(sortedList, sortedList + sortedListPtr,
            [](Renderable* const & a, Renderable* const & b) {
        return a->firstAppearTime > b->firstAppearTime;
    });
    sortedList[sortedListPtr++] = main;
    for(int i = 0; i < COLLECTOR_THREAD; i++) {
        threads[i]->start();
    }
}

float FootageCollector::finished() {
    sync.lock();
    float ret = std::min(finishedTime[0], finishedTime[1]);
    sync.unlock();
    return ret;
}

