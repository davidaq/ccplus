#pragma once
#include "global.hpp"

#include <map>

// @ Context objects contains data that should be shared between process stages.
// @ This class should not be inherited
class CCPlus::Context : public CCPlus::Object {
public:
    Context(const std::string& storagePath, int _fps);
    ~Context();
    
    // operations
    
    
    // access
    const std::string& getStoragePath() const;

    Renderable* getRenderable(const std::string& uri);
    void putRenderable(const std::string& uri, Renderable*);
    bool hasRenderable(const std::string&) const;
    int numberOfRenderable() const;
    int getFPS() const;

    void setInputDir(const std::string& dir);
    const std::string& getInputDir() const;

    // inquery

private:
    
    // data
    std::string storagePath;
    
    std::map<std::string, Renderable*> renderables;

    int fps;

    std::string inputDir = "";

    // operations
};
