#pragma once
#include "global.hpp"

#include <map>

// @ Context objects contains data that should be shared between process stages.
// @ This class should not be inherited
// @ This class is singleton
namespace CCPlus {
    struct ExtraContext;
};
class CCPlus::Context : public CCPlus::Object {
public:

    void init(const std::string& storagePath, int _fps);
    static CCPlus::Context* getInstance();
    ~Context();

    void releaseMemory();

    void setStoragePath(const std::string& path);
    const std::string& getStoragePath() const;
    void setFPS(int fps);
    int getFPS() const;

    Renderable* getRenderable(const std::string& uri);
    void putRenderable(const std::string& uri, Renderable*);
    bool hasRenderable(const std::string&) const;
    int numberOfRenderable() const;

    void setInputDir(const std::string& dir);
    const std::string& getInputDir() const;
    

    CCPlus::ExtraContext& getExtra() {
        return *extra;
    }
    // inquery
private:
    Context();

    CCPlus::ExtraContext *extra;
    // data
    std::string storagePath = "tmp";
    
    std::map<std::string, Renderable*> renderables;

    int fps = 18;

    std::string inputDir = "";
};
