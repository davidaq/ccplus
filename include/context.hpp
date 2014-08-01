#pragma once
#include "global.hpp"

#include <map>

// @ Context objects contains data that should be shared between process stages.
// @ This class should not be inherited
class CCPlus::Context : public CCPlus::Object {
public:
    Context(const std::string& _storagePath, int _fps) : 
        storagePath(_storagePath),
        fps(_fps) {}

    ~Context() {}
    
    // operations
    
    
    // access
    const std::string& getStoragePath() const {
        return storagePath;
    }

    CCPlus::Renderable* getRenderable(const std::string& uri) {
        return renderables.at(uri);
    }

    void putRenderable(const std::string& uri, CCPlus::Renderable* renderable); 

    bool hasRenderable(const std::string& uri) const {
        return (renderables.find(uri) != renderables.end());
    }

    int numberOfRenderable() const {
        return renderables.size();
    }

    int getFPS() const {
        return fps;
    }

    // inquery

private:
    
    // data
    std::string storagePath;
    
    std::map<std::string, Renderable*> renderables;

    int fps;

    // operations
};
