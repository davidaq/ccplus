#include "global.hpp"

class CCPlus::Layer {

public:
    Layer(); 

    float getTime() const;
    float getDuration() const;
    float getStart() const;
    float getLast() const;
};
