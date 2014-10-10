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

void Object::wait() {
    if(!hasSem) sem_init(&semaphore, 0, 0); hasSem = true;
    sem_wait(&semaphore);
}

void Object::notify() {
    if(!hasSem) sem_init(&semaphore, 0, 0); hasSem = true;
    sem_post(&semaphore);
}

void Object::notifyAll() {
    if(!hasSem) sem_init(&semaphore, 0, 0); hasSem = true;
    int waits;
    sem_getvalue(&semaphore, &waits);
    while(waits-- > 0) {
        sem_post(&semaphore);
    }
}

