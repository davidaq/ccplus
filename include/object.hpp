#pragma once

#include "global.hpp"
#include <iostream>
#include <set>
#include <pthread.h>
#include <semaphore.h>

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
    Semaphore(std::string name = "");
    ~Semaphore();
    void wait();
    void notify();
    void notifyAll();
private:
    sem_t* sem;
    bool named;
    std::string name;
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

