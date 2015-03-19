#pragma once
#include "global.hpp"
#include <list>
#include <functional>
#include <pthread.h>
#include <semaphore.h>

// Acts as an thread pool, but takes the current thread as a worker thread
class CCPlus::ParallelExecutor {
public:
    static pthread_t runInNewThread(std::function<void()> job);

    ParallelExecutor(int threadCount);
    ~ParallelExecutor();
    void execute(std::function<void()> job);
    void waitForAll(bool destroy=true);
private:
    pthread_t* extraThreads = 0;
    int extraThreadsCount = 0;
    int busyExtraThread = 0;
    bool ended = false;
    std::list<std::function<void()> > jobList;
    CCPlus::Lock jobListLock;
    CCPlus::Semaphore listSemaphore;
    
    static void* executeFunc(void* ctx);
    static void* threadFunc(void* ctx);
};

#ifdef __DARWIN__
#define THREAD_NAME(x) pthread_setname_np(#x)
#else
#define THREAD_NAME(x)
#endif
