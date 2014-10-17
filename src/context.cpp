#include "context.hpp"
#include "tmlreader.hpp"
#include "footage-collector.hpp"
#include "glprogram-manager.hpp"
#include "platform.hpp"

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
    if(!active)
        return;
    renderables.clear();
    delete collector;
    collector = nullptr;
    deleteRetained();

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

FT_Face& Context::fontFace() {
    static FT_Face face;
    static FT_Library ft;
    static bool init = true;
    if(init) {
        int fterror;
        fterror = FT_Init_FreeType(&ft);
        if(fterror) {
            log(logFATAL) << "Can't initialize FreeType";
        }
        cv::Mat fontData = readAsset("font.ttf");
        fterror = FT_New_Memory_Face(ft, (const unsigned char*) fontData.data,
                fontData.total(), 0, &face);
        if(fterror) {
            log(logFATAL) << "Can't load font...";
        }
        init = true;
    }
    return face;
}
