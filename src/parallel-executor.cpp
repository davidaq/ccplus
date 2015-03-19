#include "parallel-executor.hpp"
#include "logger.hpp"

using namespace CCPlus;

void* ParallelExecutor::executeFunc(void* ctx) {
    ParallelExecutor& executor = *((ParallelExecutor*)ctx);
    while(true) {
        executor.listSemaphore.wait();
        std::function<void() > job;
        bool run = false;
        executor.jobListLock.lock();
        if(!executor.jobList.empty()) {
            run = true;
            job = executor.jobList.front();
            executor.jobList.pop_front();
        }
        executor.jobListLock.unlock();
        if(run) {
            executor.jobListLock.lock();
            executor.busyExtraThread++;
            executor.jobListLock.unlock();
            job();
            executor.jobListLock.lock();
            executor.busyExtraThread--;
            executor.jobListLock.unlock();
        } else if(executor.ended) {
            break;
        }
    }
    return 0;
}

ParallelExecutor::ParallelExecutor(int threadCount) {
    extraThreadsCount = threadCount - 1;
    if(extraThreadsCount > 0) {
        extraThreads = new pthread_t[extraThreadsCount];
        for(int i = 0; i < extraThreadsCount; i++) {
            if (pthread_create(extraThreads + i, 0, 
                        ParallelExecutor::executeFunc, this)) {
                log(logFATAL) << "Failed to create new thread";
            }
        }
    }
}

ParallelExecutor::~ParallelExecutor() {
    waitForAll();
    if(extraThreads)
        delete [] extraThreads;
}

void ParallelExecutor::execute(std::function<void()> job) {
    bool runNow = false;
    jobListLock.lock();
    if(jobList.size() < extraThreadsCount * 2) {
        jobList.push_back(job);
        listSemaphore.notify();
    } else
        runNow = true;
    jobListLock.unlock();
    if(runNow)
        job();
}

void ParallelExecutor::waitForAll(bool destroy) {
    if(ended)
        return;
    if(destroy) {
        ended = true;
        listSemaphore.discard();
        executeFunc(this);
        for(int i = 0; i < extraThreadsCount; i++) 
            pthread_join(extraThreads[i], 0);
    } else if(extraThreadsCount > 0){
        listSemaphore.notifyAll();
        while(true) {
            std::function<void() > job;
            bool run = false;
            jobListLock.lock();
            if(!jobList.empty()) {
                run = true;
                job = jobList.front();
                jobList.pop_front();
            }
            jobListLock.unlock();
            if(run) {
                job();
            } else {
                break;
            }
        }
        while(true) {
            jobListLock.lock();
            bool busy = busyExtraThread > 0;
            jobListLock.unlock();
            if(!busy)
                break;
            usleep(10000);
        }
    }
}

pthread_t ParallelExecutor::runInNewThread(std::function<void()> job) {
    pthread_t thread;
    std::function<void()>* jobPtr = new std::function<void()>();
    *jobPtr = job;
    pthread_create(&thread, 0, threadFunc, jobPtr);
    return thread;
}

void* ParallelExecutor::threadFunc(void* ctx) {
    std::function<void()> &jobPtr = *((std::function<void()>*) ctx);
    jobPtr();
    delete &jobPtr;
    return 0;
}

