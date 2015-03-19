#include "context.hpp"
#include "tmlreader.hpp"
#include "footage-collector.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "platform.hpp"
#include "ccplus.hpp"
#include "image-renderable.hpp"

using namespace CCPlus;

Context* singleton = 0;

Context* Context::getContext() {
    if(!singleton)
        singleton = new Context();
    return singleton;
}

void Context::begin(const std::string& tmlPath, const std::string& footageDir) {
    if(active) {
        log(logFATAL) << "Previous context still active, context begin failed";
        return;
    }
    if(footageDir.empty())
        this->footageDir = dirName(tmlPath);
    else
        this->footageDir = footageDir;
    
    TMLReader reader;
    profile(TMLRead) {
        mainComposition = reader.read(tmlPath);
    }
    collector = new FootageCollector(mainComposition);
    flags.clear();
    active = true;

    std::srand(1421121622);
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
    bgmVolumes.clear();
    active = false;
}

std::string Context::getStoragePath(const std::string& relativePath) {
    return generatePath(outputPath, relativePath);
}

std::string Context::getFootagePath(const std::string& relativePath) {
    return generatePath(this->footageDir, relativePath);
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
