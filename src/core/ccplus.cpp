#include "global.hpp"
#include "ccplus.hpp"

void CCPlus::go(
        const std::string& tmlpath,
        const std::string& storagePath, 
        float start = 0,
        float length = 5,
        int fps = 18) {

    CCPlus::Context* ctx = new CCPlus::Context(storagePath, fps);   
    CCPlus::TMLReader* reader = new CCPlus::TMLReader(ctx);
    ctx->retain(reader);

    CCPlus::Composition* mainComp = reader->read(tmlpath);
    ctx->retain(mainComp);

    // TODO: support multi-thread rendering
    std::vector<CCPlus::CompositionDependency> deps = mainComp->fullOrderedDependency(start, length); 
    for (auto& dep : deps)
        dep.renderable->render(dep.from, dep.to);

    float inter = 1.0 / fps;
    for (float i = 0.0; i < length; i += inter) {
        float t = start + i;
        // getFrame(t) 
        // Bluh bluh
    }

    //TODO 
    // Remix audio and image seqs
    delete ctx;
}
