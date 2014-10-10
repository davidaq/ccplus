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

class CCPlus::Semaphore {
public:
    Semaphore(std::string name = "");
    ~Semaphore();
    void wait();
    void notify();
    void notifyAll();
private:
    sem_t* sem;
};

class CCPlus::Object {
public:
    virtual ~Object();
    
    void retain(Object* obj);
private:
    void unretain(Object* obj);
    
    Semaphore semaphore;
    Object* retainer = 0;
    std::set<Object*> retains;
};
