#pragma once
#include "animated-renderable.hpp"
#include "layer.hpp"
#include "pthread.h"

namespace CCPlus {
    class CompositionDependency;
    class Composition;
}

class CCPlus::CompositionDependency {
public:
    Renderable* renderable;
    float from, to;
};

class CCPlus::Composition : public CCPlus::AnimatedRenderable {

public:
    Composition(const std::string&, float, int, int);
    ~Composition();

    void renderPart(float start, float duration);
    
    void setForceRenderToFile(bool renderToFile);
    // access
    const std::string& getName() const; 

    void putLayer(const Layer&);
    std::vector<Layer> getLayers() const;
    
    std::vector<CompositionDependency> directDependency(float from, float to) const;
    std::vector<CompositionDependency> fullOrderedDependency(float from, float to) const;

    int getWidth() const;
    int getHeight() const;
    float getDuration() const;

    int getTotalNumberOfFrame() const;
    const std::string getPrefix() const;

    bool finished(int f) const {
        return rendered.count(f) != 0;
    }

private:
    /**
     * Compare layer set at two different time step.
     * It's IMPORTANT for rendering still image
     */
    bool still(float t1, float t2);

    // data
    std::string name = "";

    std::vector<Layer> layers;

    int width, height;
    float duration;
    bool renderToFile = false;

    pthread_mutex_t renderedLock;
};
