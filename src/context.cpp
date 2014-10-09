#include "extra-context.hpp"
#include "file-manager.hpp"
#include "mat-cache.hpp"

#include <OpenGL/gl.h>
#include <OpenGL/OpenGl.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>

using namespace CCPlus;

Context::Context() {
    FileManager::getInstance();

    extra = new ExtraContext;
    // Init FreeType
    int fterror;
    fterror = FT_Init_FreeType(&(extra->freetype));
    if ( fterror ) {
        log(logFATAL) << "Can't initialize FreeType";
    }
    fterror = FT_New_Memory_Face(extra->freetype,
        (const unsigned char*)
#include "res/font.ttf"
    , 
#include "res/font.ttf.count"
    , 0, &(extra->font));
    if (fterror) {
        log(logFATAL) << "Can't load font...";
    }

    /* OpenGL initialization: only for testing */
    CGLContextObj ctx;
    CGLPixelFormatAttribute attributes[4] = {
        kCGLPFAAccelerated,   
        kCGLPFAOpenGLProfile, 
        (CGLPixelFormatAttribute) kCGLOGLPVersion_Legacy,
        (CGLPixelFormatAttribute) 0
    };
    CGLPixelFormatObj pix;
    CGLError errorCode;
    GLint num; // stores the number of possible pixel formats
    errorCode = CGLChoosePixelFormat( attributes, &pix, &num );
    errorCode = CGLCreateContext( pix, NULL, &ctx ); // second parameter can be another context for object sharing
    CGLDestroyPixelFormat( pix );

    errorCode = CGLSetCurrentContext( ctx );
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
}

Context::~Context() {
    FT_Done_Face(extra->font);
    FT_Done_FreeType(extra->freetype);
    delete extra;
}

Context* Context::getInstance() {
    static Context ctx;
    return &ctx;
}

void Context::init(const std::string& _storagePath, int _fps) {
    this->storagePath = _storagePath;
    this->fps = _fps;
    this->inputDir = "";
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

void Context::setStoragePath(const std::string& path) {
    this->storagePath = path;
}

void Context::setFPS(int fps) {
    this->fps = fps;
}
