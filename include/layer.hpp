#pragma once
#include "global.hpp"

class CCPlus::Layer : public CCPlus::Object {

public:
    Layer();
    /*
     * TODO: Verify start and time
     */
    Layer(
        const std::string& renderableUri, 
        float time, 
        float duration, 
        float start, 
        float last,
        int width,
        int height,
        int blendMode = 0,
        int trkMat = 0,
        bool showup = true
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

    /*
     * map layer time to layer renderable item local time
     */
    float mapInnerTime(float time) const;

    int getBlendMode() const;
    int getTrackMatte() const;

    /*
     * Check whether this layer is visible 
     * at the timestep @t
     * Note: the time @t are relative to upper layer
     */
    bool visible(float t) const;
    /*
     * Show is to check whether this layer is visible
     * FIXME: this will cause confusion with visible
     */
    bool show() const;

    // Assume the renderable stuff is rendered
    Frame applyFiltersToFrame(float); 

    void setProperties(const std::map<std::string, Property>&);
    void setProperties(const std::map<std::string, Property>&,
            const std::vector<std::string>& keyOrder);
    const std::map<std::string, Property>& getProperties() const;

    std::vector<float> interpolate(const std::string&, float) const;
private:
    // data
    std::string renderableUri;
    CCPlus::Renderable* renderObject = 0;
    
    float time = 0, duration = 0, start = 0, last = 0;
    int width = 0;
    int height = 0;
    int blendMode = 0;
    int trkMat = 0;
    bool showup = true;
    std::map<std::string, Property> properties;
    std::vector<std::string> orderedKey;
};
