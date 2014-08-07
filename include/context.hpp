#pragma once
#include "global.hpp"

#include <map>

namespace CCPlus {
    class FileManager;
};

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

    CCPlus::FileManager* getFileManager();

    // inquery

private:
    
    // data
    std::string storagePath;
    
    std::map<std::string, Renderable*> renderables;

    int fps;

    std::string inputDir = "";

    CCPlus::FileManager* fileManager;

    // operations
};
