#include "global.hpp"
#include "ccplus.hpp"
#include "utils.hpp"

//using namespace CCPlus;



void* initContext(const char* storagePath, int fps) {
    //Context* ctx = new Context(storagePath, fps);
    //TMLReader reader(ctx);
    //reader.read(tmlPath);
    //return (void*) ctx;
}

void CCPlus::go(
        const std::string& tmlpath,
        const std::string& storagePath, 
        float start,
        float length,
        int fps) {

    CCPlus::Context* ctx = new CCPlus::Context(storagePath, fps);   
    CCPlus::TMLReader* reader = new CCPlus::TMLReader(ctx);
    ctx->retain(reader);

    CCPlus::Composition* mainComp = reader->read(tmlpath);
    ctx->retain(mainComp);

    // TODO: support multi-thread rendering
    std::vector<CCPlus::CompositionDependency> deps = mainComp->fullOrderedDependency(start, length); 
    for (auto& dep : deps) {
        dep.renderable->render(dep.from, dep.to);
    }

    float inter = 1.0 / fps;
    for (float i = 0.0; i < length; i += inter) {
        float t = start + i;
        Image img = mainComp->getFrame(t);
        img.write(generatePath(storagePath, "test" + std::to_string(i) + ".jpg"));
        // getFrame(t) 
        // Bluh bluh
    }

    //TODO 
    // Remix audio and image seqs
    delete ctx;
}
