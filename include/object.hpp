#pragma once

#include "global.hpp"
#include <iostream>
#include <set>
#include <pthread.h>

#ifdef __DARWIN__
#include <dispatch/dispatch.h>
#define os_sem_t dispatch_semaphore_t
#else
#include <semaphore.h>
#define os_sem_t sem_t
#endif

class CCPlus::Lock {
public:
    Lock();
    void lock();
    void unlock();
private:
    pthread_mutex_t mutex;
};

class CCPlus::ScopeHelper {
public:
    ScopeHelper(std::function<void()> action);
    ~ScopeHelper();
    operator bool();
private:
    std::function<void()> action;
    int passed = 0;
};


class CCPlus::Semaphore {
public:
    Semaphore();
    ~Semaphore();
    void wait();
    void notify();
    void notifyAll();
    void discard();
private:
    Lock lock;
    bool discarded = false;
    int waits = 0;
    os_sem_t sem;
};

class CCPlus::Object {
public:
    virtual ~Object();
    
    void retain(Object* obj);
    void deleteRetained();
private:
    void unretain(Object* obj);
    
    Object* retainer = 0;
    std::set<Object*> retains;
};

