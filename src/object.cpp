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

Semaphore::Semaphore(std::string name) {
#ifdef __OSX__
    if(name == "") {
        static int counter = 0;
        char sname[20];
        sprintf(sname, "annonymous%d", counter++);
        name = sname;
    }
    name = "CCPLUS_" + name;
    sem_unlink(name.c_str());
    sem = sem_open(name.c_str(), O_CREAT, 0655, 0);
#else
    sem = (sem_t*)-1;
#endif
    if(sem == (sem_t*)-1) {
        sem = new sem_t;
        sem_init(sem, 0, 0);
        named = false;
    } else {
        named = true;
    }
}

Semaphore::~Semaphore() {
#ifdef __OSX__
    sem_close(sem);
#endif
    if(!named) {
        sem_destroy(sem);
        delete sem;
    }
}

void Semaphore::wait() {
    sem_wait(sem);
    usleep(10000);
}

void Semaphore::notify() {
    sem_post(sem);
}

void Semaphore::notifyAll() {
    int waits;
    sem_getvalue(sem, &waits);
    while(waits-- > 0) {
        sem_post(sem);
    }
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

