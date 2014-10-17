#pragma once

#include "global.hpp"

/*
 * Calculate dependency 
 * Use multi-thread to prepare footages
 * Return finished time point
 */
namespace CCPlus {
    class CollectorThread;
}
class CCPlus::FootageCollector : public CCPlus::Object {
public:
    FootageCollector(CCPlus::Composition* main);
    ~FootageCollector();

    void prepare();
    void stop();
    // Return the time point before that all preparation was done
    float finished();

    CCPlus::Semaphore signal;
    float finishedTime[COLLECTOR_THREAD];
    CollectorThread* threads[COLLECTOR_THREAD] = {0};
    CCPlus::Composition* main = 0;
    Renderable** sortedList = 0;
    int sortedListPtr;

    // wait if prepared over limit
    float limit;
    Lock sync;
};
