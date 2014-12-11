#pragma once
#include "global.hpp"

#include <map>

// @ Context objects contains data that should be shared between process stages.
// @ This class should not be inherited
namespace CCPlus {
    struct ExtraContext;
};
class CCPlus::Context : public CCPlus::Object {
public:
    Context(const std::string& storagePath, 
            int _fps);
    ~Context();

    void releaseMemory();

    // access
    const std::string& getStoragePath() const;

    Renderable* getRenderable(const std::string& uri);
    void putRenderable(const std::string& uri, Renderable*);
    bool hasRenderable(const std::string&) const;
    int numberOfRenderable() const;
    int getFPS() const;

    void setInputDir(const std::string& dir);
    const std::string& getInputDir() const;

    CCPlus::ExtraContext& getExtra() {
        return *extra;
    }
    // inquery
private:
    CCPlus::ExtraContext *extra;
    // data
    std::string storagePath;
    
    std::map<std::string, Renderable*> renderables;

    int fps;

    std::string inputDir = "";
};
