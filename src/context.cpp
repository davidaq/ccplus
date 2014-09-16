#include "extra-context.hpp"
#include "file-manager.hpp"
#include "mat-cache.hpp"

using namespace CCPlus;

Context::Context(const std::string& _storagePath, 
        int _fps, bool _enableGPU):
    storagePath(_storagePath), fps(_fps)
{
    FileManager::getInstance();
    
    if (_enableGPU) {
        gpu = new GPUWorker();
    }
    
    extra = new ExtraContext;
    // Init FreeType
    int fterror;
    fterror = FT_Init_FreeType(&(extra->freetype));
    if ( fterror ) {
        log(logFATAL) << "Can't initialize FreeType";
    }
    // TODO pack real font
    fterror = FT_New_Memory_Face(extra->freetype,
        (const unsigned char*)
#include "res/font.ttf"
    , 
#include "res/font.ttf.count"
    , 0, &(extra->font));
    if (fterror) {
        log(logFATAL) << "Can't load font...";
    }
}

Context::~Context() {
    if (gpu)
        delete gpu;
    FT_Done_Face(extra->font);
    FT_Done_FreeType(extra->freetype);
    delete extra;
}

void Context::releaseMemory() {
    FileManager::getInstance()->clear();
    for (auto& r : renderables) {
        r.second->clear();
    }
    MatCache::clear();
}

const std::string& Context::getStoragePath() const {
    return storagePath;
}

Renderable* Context::getRenderable(const std::string& uri) {
    return renderables.at(uri);
}

void Context::putRenderable(const std::string& uri, Renderable* renderable) {
    this->retain(renderable);
    renderables[uri] = renderable;
}

bool Context::hasRenderable(const std::string& uri) const {
    return (renderables.find(uri) != renderables.end());
}

int Context::numberOfRenderable() const {
    return renderables.size();
}

int Context::getFPS() const {
    return fps;
}

void Context::setInputDir(const std::string& dir) {
    this->inputDir = dir;
}

const std::string& Context::getInputDir() const {
    return inputDir;
}

GPUWorker* Context::getGPUWorker() {
    return gpu;
}

bool Context::enableGPU() const {
    return gpu != 0;
}
