#pragma once
#include "global.hpp"

#include <map>

// @ Context objects contains data that should be shared between process stages.
// @ This class should not be inherited
class CCPlus::Context : public CCPlus::Object {
public:
    Context(const std::string& storagePath);
    ~Context();
    
    // operations
    
    
    // access
    const std::string& getStoragePath() const;

    Renderable* getRenderable(std::string uri);
    void putRenderable(std::string uri, Renderable*);

    // inquery

private:
    
    // data
    std::string storagePath;
    
    std::map<std::string, Renderable*> renderables;
    
    // operations
};