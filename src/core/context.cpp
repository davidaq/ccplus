#include "context.hpp"

using namespace CCPlus;

Context::Context(const std::string& _storagePath):
    storagePath(_storagePath)
{
    // TODO make sure storagePath directory exists
}

Context::~Context() {
    // TODO clean up renderables' memory
    // TODO delete storagePath directory
}

const std::string& Context::getStoragePath() const {
    return storagePath;
}

Renderable* Context::getRenderable(std::string uri) {
    return renderables.at(uri);
}

void Context::putRenderable(std::string uri, Renderable* renderable) {
    renderables[uri] = renderable;
}

bool Context::hasRenderable(std::string uri) {
    return (renderables.find(uri) != renderables.end());
}
