#pragma once
#include "global.hpp"

struct DoubleLess {
    bool operator() (float left, float right) const
    {
        //return left < right;
        return (std::abs(left - right) > 0.00001) && (left < right);
    }
}; 
typedef std::map<float, std::vector<float>, DoubleLess> Property;
typedef std::map<std::string, Property> PropertyMap;

class CCPlus::Layer : public CCPlus::Object {

public:
    Layer();
    Layer(
        const std::string& renderableUri, 
        float time,      // time in the parent composition where this layer starts
        float duration,  // total appearance time of this layer in parent scale
        float start,     // the begining time of the refered renderable
        float last,      // total cliped length of the refered renderable
        int width,
        int height,
        int blendMode = 0,
        int trkMat = 0,
        bool showup = true
    );

    CCPlus::Renderable* getRenderObject(); 
    /*
     * @time and @duration are to upper layer 
     * comp's timeline
     * @duration is length
     */
    float time, duration;
    /*
     * Here @start and @last are to local timeline of @Renderable object
     * @last is length
     */
    float start, last;

    int blendMode = 0;
    int trkMat = 0;

    /*
     * map layer time to layer renderable item local time
     */
    float mapInnerTime(float time) const;

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
    bool show = true;

    // Assume the renderable stuff is rendered
    void applyFiltersToFrame(GPUFrame& frame, GPUFrame& buffer, float time); 

    void setProperties(const std::map<std::string, Property>&,
            const std::vector<std::string>& keyOrder);
    const std::map<std::string, Property>& getProperties() const;

    std::vector<float> interpolate(const std::string&, float) const;
private:
    // data
    std::string renderableUri;
    CCPlus::Renderable* renderObject = 0;
    
    int width = 0;
    int height = 0;
    std::map<std::string, Property> properties;
    std::vector<std::string> orderedKey;
};
