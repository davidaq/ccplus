#pragma once

#include "global.hpp"

/*
 * Calculate dependency 
 * Use multi-thread to prepare footages
 * Return finished time point
 * Release footage based on render progress.
 */
class CCPlus::FootageCollector : public CCPlus::Object {
public:
    FootageCollector(CCPlus::Composition* main);
    ~FootageCollector();

    void prepare();
    void stop();

    // Clean all footage before @time
    void clean(float time);
    float finished();

    CCPlus::Semaphore signal;
    float renderTime = 0;
    float windowDuration = CCPlus::collectorTimeInterval * 4;
    //Lock renderTimeLock;
    float finishedTime;
    CCPlus::Composition* main = 0;
    bool continueRunning = true;

    Lock sync;
private:
    void doPrepare();
};
