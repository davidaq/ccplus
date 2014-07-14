#pragma once
#include "global.hpp"

class CCPlus::Layer : public CCPlus::Object {

public:
    Layer();
    Layer(
        Context* ctx,
        const std::string& renderableUri, 
        float time, 
        float duration, 
        float start, 
        float last
    );

    // access
    CCPlus::Renderable* getRenderObject();
    float getTime() const;
    float getDuration() const;
    float getStart() const;
    float getLast() const;

    void setProperties(const std::map<std::string, Property>&);
    std::map<std::string, Property> getProperties() const;

    std::vector<float> interpolate(const std::string&, float) const;
private:
    // data
    Context* context;
    std::string renderableUri;
    CCPlus::Renderable* renderObject = 0;
    
    float time = 0, duration = 0, start = 0, last = 0;
    std::map<std::string, Property> properties;
};