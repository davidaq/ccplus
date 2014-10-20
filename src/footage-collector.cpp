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
        thread = ParallelExecutor::runInNewThread([&]() {
            bool goon = true;
            float fStep = 1.0 / Context::getContext()->fps;
            while(Context::getContext()->isActive() && goon) {
                c.sync.lock();
                if(c.sortedListPtr > 0) {
                    Renderable* pitem = c.sortedList[--c.sortedListPtr];
                    float t = c.finishedTime[index] = pitem->firstAppearTime - fStep;
                    c.sync.unlock();
                    if(!pitem->usedFragments.empty()) {
                        log(logINFO) << "prepare begin" << pitem->getUri();
                        pitem->prepare();
                        log(logINFO) << "prepare end" << pitem->getUri();
                    }
                    while(t > c.limit)
                        sleep(1);
                    c.sync.lock();
                } else {
                    c.finishedTime[index] = c.main->duration + 1;
                    goon = false;
                }
                c.sync.unlock();
                c.signal.notify();
            }
        });
    }

    void stop() {
        if (thread) 
            pthread_join(thread, 0);
    }
private:
    int index;
    FootageCollector& c;
    pthread_t thread = 0;
};

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
    for(int i = 0; i < COLLECTOR_THREAD; i++) {
        finishedTime[i] = 0;
        threads[i] = new CollectorThread(i, this);
    }
}

FootageCollector::~FootageCollector() {
    // Prevent dead-lock of footage collector
    limit = 0x7fffffff;
    stop();
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
        if(ite.second && ite.second != main)
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

void FootageCollector::stop() {
    for (int i = 0; i < COLLECTOR_THREAD; i++) {
        if (threads[i])
            threads[i]->stop();
    }
}

float FootageCollector::finished() {
    sync.lock();
    float ret = main->duration + 1;
    for(int i = 0; i < COLLECTOR_THREAD; i++) {
        if(finishedTime[i] < ret)
            ret = finishedTime[i];
    }
    sync.unlock();
    return ret;
}

