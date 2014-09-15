#include "context.hpp"
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
}

Context::~Context() {
    if (gpu)
        delete gpu;
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
