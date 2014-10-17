#include "context.hpp"
#include "tmlreader.hpp"
#include "footage-collector.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "platform.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

Context* singleton = 0;

Context* Context::getContext() {
    if(!singleton)
        singleton = new Context();
    return singleton;
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

    L() << this;
    
    TMLReader reader;
    mainComposition = reader.read(tmlPath);
    collector = new FootageCollector(mainComposition);
    flags.clear();
    active = true;
}

void Context::end() {
    if (collector) {
        delete collector;
        collector = nullptr;
    }
    renderables.clear();
    deleteRetained();
    GPUFrameCache::clear();
    GLProgramManager::getManager()->clean();
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

FT_Library& Context::freetype() {
    if(!freetypeInited) {
        freetypeInited = true;
        int fterror;
        fterror = FT_Init_FreeType(&ft);
        if(fterror) {
            log(logFATAL) << "Can't initialize FreeType";
        } else {
            freetypeInited = true;
        }
    }
    return ft;
}
