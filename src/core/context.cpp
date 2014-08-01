#include "context.hpp"

using namespace CCPlus;

void Context::putRenderable(const std::string& uri, Renderable* renderable) {
    this->retain(renderable);
    renderables[uri] = renderable;
}
