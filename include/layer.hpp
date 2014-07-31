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
        float last,
        int width,
        int height
    );

    // access
    CCPlus::Renderable* getRenderObject(); 
    /*
     * @time and @duration are to upper layer 
     * comp's timeline
     * @duration is length
     */
    float getTime() const;
    float getDuration() const;
    /*
     * Here @start and @last are to local timeline of @Renderable object
     * @last is length
     */
    float getStart() const;
    float getLast() const;

    // Assume the renderable stuff is rendered
    Frame applyFiltersToFrame(float); 

    void setProperties(const std::map<std::string, Property>&);
    void setProperties(const std::map<std::string, Property>&,
            const std::vector<std::string>& keyOrder);
    std::map<std::string, Property> getProperties() const;

    std::vector<float> interpolate(const std::string&, float) const;
private:
    // data
    Context* context;
    std::string renderableUri;
    CCPlus::Renderable* renderObject = 0;
    
    float time = 0, duration = 0, start = 0, last = 0;
    int width = 0;
    int height = 0;
    std::map<std::string, Property> properties;
    std::vector<std::string> orderedKey;
};
