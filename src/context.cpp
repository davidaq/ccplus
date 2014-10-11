#include "context.hpp"
#include "tmlreader.hpp"
#include "footage-collector.hpp"

using namespace CCPlus;

Context* Context::getContext() {
    static Context context;
    return &context;
}

void Context::begin(const std::string& tmlPath, const std::string& storagePath, int fps) {
    if(active) {
        log(logFATAL) << "Previous context still active, context begin failed";
        return;
    }
    end();
    this->tmlDir = dirName(tmlPath);
    this->storagePath = storagePath;
    this->fps = fps;
    
    TMLReader reader;
    mainComposition = reader.read(tmlPath);
    collector = new FootageCollector(mainComposition);
    flags.clear();
    active = true;
}

void Context::end() {
    if(!active)
        return;
    renderables.clear();
    delete collector;
    collector = nullptr;
    deleteRetained();
    active = false;
}

std::string Context::getStoragePath(const std::string& relativePath) {
    return generatePath(this->storagePath, relativePath);
}

std::string Context::getFootagePath(const std::string& relativePath) {
    return generatePath(this->tmlDir, relativePath);
}

bool Context::hasRenderable(const std::string& uri) {
    return renderables.count(uri);
}

void Context::putRenderable(const std::string& uri, Renderable* renderable) {
    renderables[uri] = renderable;
}

Renderable* Context::getRenderable(const std::string& uri) {
    return renderables[uri];
}
