#include "parallel-executor.hpp"
#include "logger.hpp"

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
    pthread_mutex_lock(&jobListLock);
    if(jobList.size() < extraThreadsCount * 2) {
        jobList.push_back(job);
        sem_post(&listSemaphore);
    } else
        runNow = true;
    pthread_mutex_unlock(&jobListLock);
    if(runNow)
        job();
}

void ParallelExecutor::waitForAll() {
    if(ended)
        return;
    ended = true;
    // Activate other sleeping thread
    for(int i = 0; i < extraThreadsCount; i++)
        sem_post(&listSemaphore);
    // Main thread start running
    sem_post(&listSemaphore);
    executeFunc(this);
    for(int i = 0; i < extraThreadsCount; i++) 
        pthread_join(extraThreads[i], 0);
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

