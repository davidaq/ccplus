#pragma once

#include "global.hpp"
#include <set>
#include <pthread.h>
#include <semaphore.h>

class CCPlus::Object {
public:
    virtual ~Object();
    
    void retain(Object* obj);
    void wait();
    void notify();
    void notifyAll();
private:
    void unretain(Object* obj);
    
    bool hasSem = false;
    sem_t semaphore;
    Object* retainer = 0;
    std::set<Object*> retains;
};
