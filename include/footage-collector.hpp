#pragma once

#include "global.hpp"

/*
 * Calculate dependency 
 * Use multi-thread to prepare footages
 * Return finished time point
 */
class CCPlus::FootageCollector : public CCPlus::Object {
public:
    FootageCollector(CCPlus::Composition* main);
    ~FootageCollector();

    void prepare();
    // Return the time point before that all preparation was done
    float finished();
    CCPlus::Semaphore signal;
private:
    CCPlus::Composition* main;
};
