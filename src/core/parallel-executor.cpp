#include "parallel-executor.hpp"

using namespace CCPlus;

void* ParallelExecutor::executeFunc(void* ctx) {
    ParallelExecutor& executor = *((ParallelExecutor*)ctx);
    while(true) {
        sem_wait(&executor.listSemaphore);
        std::function<void() > job;
        bool run = false;
        pthread_mutex_lock(&executor.jobListLock);
        if(!executor.jobList.empty()) {
            run = true;
            job = executor.jobList.front();
            executor.jobList.pop_front();
        }
        pthread_mutex_unlock(&executor.jobListLock);
        if(run) {
            job();
        } else if(executor.ended) {
            break;
        }
    }
    return 0;
}

ParallelExecutor::ParallelExecutor(int threadCount) {
    extraThreadsCount = threadCount - 1;
    pthread_mutex_init(&jobListLock, 0);
    sem_init(&listSemaphore, 0, 0);
    if(extraThreadsCount > 0) {
        extraThreads = new pthread_t[extraThreadsCount];
        for(int i = 0; i < extraThreadsCount; i++) {
            pthread_create(extraThreads + i, 0, ParallelExecutor::executeFunc, this);
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
    pthread_mutex_lock(&jobListLock);
    if(jobList.size() < extraThreadsCount * 2) 
        jobList.push_back(job);
    else
        runNow = true;
    sem_post(&listSemaphore);
    pthread_mutex_unlock(&jobListLock);
    if(runNow)
        job();
}

void ParallelExecutor::waitForAll() {
    if(ended)
        return;
    ended = true;
    for(int i = 0; i < extraThreadsCount; i++)
        sem_post(&listSemaphore);
    for(int i = 0; i < extraThreadsCount; i++)
        pthread_join(extraThreads[i], 0);
}

