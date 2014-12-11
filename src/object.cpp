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
