#include "context.hpp"

using namespace CCPlus;

Context* Context::getContext() {
    static Context context;
    return &context;
}

void Context::begin(const std::string& tmlPath, const std::string& storagePath, int fps) {
    this->tmlDir = dirName(tmlPath);
    this->storagePath = storagePath;
    this->fps = fps;
}

void Context::end() {
}

std::string Context::getStoragePath(const std::string& relativePath) {
    return generatePath(this->storagePath, relativePath);
}

std::string Context::getFootagePath(const std::string& relativePath) {
    return generatePath(this->tmlDir, relativePath);
}
