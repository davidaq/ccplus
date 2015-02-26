#include "object.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace CCPlus;

Object::~Object() {
    deleteRetained();
}

void Object::deleteRetained() {
    for(Object* obj : retains) {
        if(obj)
            delete obj;
    }
    retains.clear();
}

void Object::retain(Object* obj) {
    if(obj == 0)
        return;
    if(obj->retainer != 0)
        obj->retainer->unretain(obj);
    obj->retainer = this;
    retains.insert(obj);
}

void Object::unretain(Object* obj) {
    retains.erase(obj);
}

Semaphore::Semaphore() {
#ifdef __DARWIN__
    sem = dispatch_semaphore_create(0);
#else
    sem_init(&sem, 0, 0);
#endif
}

Semaphore::~Semaphore() {
    discard();
}

void Semaphore::discard() {
    if(discarded)
        return;
    discarded = true;
    notifyAll();
#ifdef __DARWIN__
    dispatch_release(sem);
#else
    sem_destroy(&sem);
#endif
}

void Semaphore::wait() {
    if(discarded)
        return;
    lock.lock();
    waits++;
    lock.unlock();
#ifdef __DARWIN__
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
#else
    sem_wait(&sem);
#endif
}

static inline void notify(os_sem_t& sem) {
#ifdef __DARWIN__
    dispatch_semaphore_signal(sem);
#else
    sem_post(&sem);
#endif
}

void Semaphore::notify() {
    lock.lock();
    if(waits > 0) {
        waits--;
        ::notify(sem);
    }
    lock.unlock();
}

void Semaphore::notifyAll() {
    lock.lock();
    while(waits --> 0) {
        ::notify(sem);
    }
    waits = 0;
    lock.unlock();
}

Lock::Lock() {
    pthread_mutex_init(&mutex, 0);
}

void Lock::lock() {
    pthread_mutex_lock(&mutex);
}

void Lock::unlock() {
    pthread_mutex_unlock(&mutex);
}

ScopeHelper::ScopeHelper(std::function<void()> action) {
    this->action = action;
}

ScopeHelper::~ScopeHelper() {
    action();
}

ScopeHelper::operator bool() {
    return passed++ == 0;
}
