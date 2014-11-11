#include "global.hpp"
#include "platform.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"
#include "composition.hpp"
#include "video-encoder.hpp"
#include "gpu-frame.hpp"
#include "render.hpp"
#include "profile.hpp"
#include "parallel-executor.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace CCPlus;

pthread_t render_thread = 0;
bool continueRunning = false;
int renderProgress = 0;

void CCPlus::go(const std::string& tmlPath, const std::string& outputPath, int fps) {
    initContext(tmlPath, outputPath, fps);
    render();
    waitRender();
    encode();
    releaseContext();
    profileFlush;
}

void CCPlus::initContext(const std::string& tmlPath, const std::string& outputPath, int fps) {
    Context::getContext()->begin(tmlPath, outputPath, fps);
    renderProgress = 0;
}

void CCPlus::releaseContext() {
    continueRunning = false;
    waitRender();
    Context* ctx = Context::getContext();
    ctx->end();
    render_thread = 0;
}

void CCPlus::deepReleaseContext() {
    releaseContext();
    Context* ctx = Context::getContext();
    for (auto& kv : ctx->preservedRenderable) {
        delete kv.second;
    }
    ctx->preservedRenderable.clear();
}

void CCPlus::render() {
    if (render_thread) {
        log(logERROR) << "Another render context is currently in use! Aborted.";
        return;
    }
    if (!Context::getContext()->isActive()) {
        log(logERROR) << "Context hasn't been initialized! Aborted rendering.";
        return;
    }
    continueRunning = true;
    render_thread = ParallelExecutor::runInNewThread([] () {
        Context* ctx = Context::getContext();
        void* glCtx = createGLContext();
        ctx->collector->limit = 10;
        ctx->collector->prepare();
        float delta = 1.0f / ctx->fps;
        float duration = ctx->mainComposition->getDuration();
        int fn = 0;
        initGL();
        GPUFrame blackBackground = GPUFrameCache::alloc(
                ctx->mainComposition->width, 
                ctx->mainComposition->height);
        blackBackground->bindFBO();
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);

        for (float i = 0; i <= duration; i += delta) {
            renderProgress = (i * 98 / duration) + 1;
            if (!continueRunning) {
                renderProgress = 0;
                log(logINFO) << "----Rendering process is terminated!---";
                return;
            }
            while(ctx->collector->finished() < i) {
                log(logINFO) << "wait --" << ctx->collector->finished();
                ctx->collector->signal.wait();
            }
            ctx->collector->limit = i + 10;
            log(logINFO) << "render frame --" << i << ':' << renderProgress << '%';
            GPUFrame frame = ctx->mainComposition->getGPUFrame(i);
            frame = mergeFrame(blackBackground, frame, DEFAULT);
            char buf[20];
            sprintf(buf, "%07d.zim", fn++);
            frame->toCPU().write(ctx->getStoragePath(buf));
            for(auto item = ctx->renderables.begin();
                    item != ctx->renderables.end(); ) {
                Renderable* r = item->second;
                if(r && !r->usedFragments.empty() && r->lastAppearTime < i) {
                    log(logINFO) << "release" << item->first;
                    r->release();
                    ctx->renderables.erase(item++);
                } else {
                    item++;
                }
            }
        }
        destroyGLContext(glCtx);
    });
}

int CCPlus::getRenderProgress() {
    return renderProgress;
}

void CCPlus::waitRender() {
    if (!render_thread) return;
    pthread_join(render_thread, 0);
}

void CCPlus::encode() {
    CCPlus::waitRender();
    Context* ctx = Context::getContext();
    if (!ctx->isActive()) {
        log(logERROR) << "Context hasn't been initialized! Aborted encoding.";
        return;
    }
    VideoEncoder encoder(
            ctx->getStoragePath("result.mp4"),
            ctx->fps);
    float delta = 1.0 / ctx->fps;
    float duration = ctx->mainComposition->getDuration();
    int fn = 0;
    for (float i = 0; i <= duration; i += delta) {
        Frame f;
        char buf[64];
        sprintf(buf, "%07d.zim", fn);
        f.read(generatePath(ctx->storagePath, buf));
        encoder.appendFrame(f);
        fn++;
    }
    encoder.finish();
    renderProgress = 100;
}

int CCPlus::numberOfFrames() {
    Context* ctx = Context::getContext();
    float d = 1.0 / ctx->fps;
    return ctx->mainComposition->getDuration() / d;
}

std::string assetPath;
void CCPlus::setAssetsPath(const std::string& path) {
    assetPath = path;
}

void CCPlus::generateCoverImages(const std::string& tmlPath, const std::string& outputPath) {
    using boost::property_tree::ptree;
    ptree pt;
    read_json(tmlPath, pt);
    int size = 0;
    for (auto& child : pt.get_child("compositions")) {
        const std::string& s = child.first.data();
        if (s[0] == '@' && s.length() > 1) {
            size++;
        }
    }

    std::string dir = dirName(tmlPath);

    for (int i = 0; i < size; i++) {
        auto& layers = pt.get_child("compositions.#COVER.layers");
        for (auto& kv : layers) {
            if (stringStartsWith(kv.second.get("uri", ""), "composition://@")) {
                kv.second.put("uri", "composition://@" + toString(i));
            }
        }
        pt.put("main", "#COVER");
        std::ofstream fileStream;
        fileStream.open(generatePath(dir, "COVER" + toString(i) + ".tml"));
        write_json(fileStream, pt, true);
    }

    for (int i = 0; i < size; i++) {
        initContext(generatePath(dir, "COVER" + toString(i) + ".tml"), outputPath, 18);
        render();
        waitRender();
        releaseContext();
        Frame f;
        f.read(generatePath(outputPath, "0000000.zim"));
        imwrite(generatePath(outputPath, toString(i) + ".jpg"), f.image);
    }
}
