#pragma once
#include "global.hpp"
#include <list>
#include <functional>
#include <pthread.h>
#include <semaphore.h>

// Acts as an thread pool, but takes the current thread as an worker thread
class CCPlus::ParallelExecutor {
public:
    ParallelExecutor(int threadCount);
    ~ParallelExecutor();
    void execute(std::function<void()> job);
    void waitForAll();
private:
    pthread_t* extraThreads = 0;
    int extraThreadsCount = 0;
    bool ended = false;
    std::list<std::function<void()> > jobList;
    pthread_mutex_t jobListLock;
    sem_t listSemaphore;
    
    static void* executeFunc(void* ctx);
};
