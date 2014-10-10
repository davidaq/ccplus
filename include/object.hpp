#pragma once

#include "global.hpp"
#include <set>

class CCPlus::Object {
public:
    virtual ~Object();
    
    void retain(Object* obj);
private:
    void unretain(Object* obj);
    
    
    Object* retainer = 0;
    std::set<Object*> retains;
};
