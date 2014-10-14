#include "footage-collector.hpp"
#include "layer.hpp"
#include "composition.hpp"
#include "context.hpp"
#include "dependency-walker.hpp"
#include "parallel-executor.hpp"

#include <algorithm>

using namespace CCPlus;

FootageCollector::FootageCollector(Composition* comp) {
    main = comp;
}

void FootageCollector::prepare() {
    DependencyWalker dep(*main);
    dep.walkThrough();
    Context* ctx = Context::getContext();
    ParallelExecutor::runInNewThread([&]() {
        ParallelExecutor executor(2);
        std::vector<Renderable*> list;
        list.reserve(ctx->renderables.size());
        for(auto ite : ctx->renderables) {
            list.push_back(ite.second);
        }
        std::sort(list.begin(), list.end(), [](Renderable* const & a, Renderable* const & b) {
            return a->firstAppearTime < b->firstAppearTime;
        });
        for(Renderable* r : list) {
            executor.execute([&]() {
                r->prepare();
                if(finishedTime < r->firstAppearTime)
                    finishedTime = r->firstAppearTime;
            });
        }
        executor.waitForAll();
        finishedTime = main->duration + 1;
    });
}

float FootageCollector::finished() {
    return finishedTime;
}

