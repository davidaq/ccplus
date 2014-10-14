#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "context.hpp"
#include "dependency-walker.hpp"

#include <algorithm>

using namespace CCPlus;

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
}

FootageCollector::~FootageCollector() {
}

void FootageCollector::prepare() {
    DependencyWalker dep(*main);
    dep.walkThrough();
    Context* ctx = Context::getContext();
    for (auto ite : ctx->renderables) {
        ite.second->prepare();
    }
}

float FootageCollector::finished() {
    return main->duration + 1;
}

