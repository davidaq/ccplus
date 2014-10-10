#pragma once
#include "global.hpp"

class CCPlus::Context : public CCPlus::Object {
public:
    static Context* getContext();

    void begin(const std::string& tmlPath, const std::string& storagePath, int fps);
    void end();
    
    std::string getFootagePath(const std::string& relativePath);
    std::string getStoragePath(const std::string& relativePath);

    std::string tmlDir = "", storagePath = "";
    std::map<std::string, Renderable*> renderables;
    int fps = 0;
};

