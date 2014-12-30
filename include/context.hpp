#pragma once
#include "global.hpp"

class CCPlus::Context : public CCPlus::Object {
public:
    static Context* getContext();

    void begin(const std::string& tmlPath);
    void end();
    
    std::string getFootagePath(const std::string& relativePath);
    std::string getStoragePath(const std::string& relativePath);
    bool hasRenderable(const std::string& uri);
    void putRenderable(const std::string& uri, Renderable* renderable);
    Renderable* getRenderable(const std::string& uri);

    std::string tmlDir = "";
    std::map<std::string, Renderable*> renderables;
    std::vector<std::pair<float, float> > bgmVolumes; // Time -> volume

    Composition* mainComposition;
    FootageCollector* collector = nullptr;

    std::set<std::string> flags;
    FT_Library& freetype();
    bool isActive() {
        return active;
    }

private:
    bool active = false;
    FT_Library ft;
    bool freetypeInited = false;
};

