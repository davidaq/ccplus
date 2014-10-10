#include "object.hpp"
#include <stdio.h>

using namespace CCPlus;

Object::~Object() {
    for(Object* obj : retains) {
        delete obj;
    }
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
    if(name == "") {
        static int counter = 0;
        char sname[20];
        sprintf(sname, "annonymous%d", counter++);
        name = sname;
    }
    name = "CCPLUS_" + name;
    sem_unlink(name.c_str());
    sem = sem_open(name.c_str(), O_CREAT, O_RDWR, 0);
}

Semaphore::~Semaphore() {
    sem_close(sem);
}

void Semaphore::wait() {
    sem_wait(sem);
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

