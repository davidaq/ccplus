#include "global.hpp"
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
bool initing = false;

void CCPlus::go(const std::string& tmlPath) {
    initContext(tmlPath);
    render();
    waitRender();
    releaseContext();
    profileFlush;
}

void CCPlus::initContext(const std::string& tmlPath) {
    releaseContext();
    initing = true;
    Context::getContext()->begin(tmlPath);
    renderProgress = 0;
    initing = false;
}

void CCPlus::releaseContext(bool forceClearCache) {
    while(initing) {
        sleep(1);
    }
    static bool releasingContext = false;
    Context* ctx = Context::getContext();
    if(releasingContext) {
        // Don't do any thing if someone else is calling release
        // just wait for end instead 
        while(releasingContext)
            sleep(1);
    } else {
        releasingContext = true;
        continueRunning = false;
        waitRender();
        ctx->end();
        render_thread = 0;
        renderProgress = 0;
        releasingContext = false;
    }
    if(forceClearCache || renderMode != PREVIEW_MODE) {
        if(releasingContext) {
            while(releasingContext)
                sleep(1);
        } else {
            releasingContext = true;
            for (auto& kv : ctx->preservedRenderable) {
                delete kv.second;
            }
            ctx->preservedRenderable.clear();
            releasingContext = false;
        }
    }
}

void renderAs(std::function<void(const Frame&)> writeFuc) {
    if (render_thread) {
        log(logERROR) << "Another render context is currently in use! Aborted.";
        return;
    }
    if (!Context::getContext()->isActive()) {
        log(logERROR) << "Context hasn't been initialized! Aborted rendering.";
        return;
    }
    continueRunning = true;
    render_thread = ParallelExecutor::runInNewThread([&writeFuc] () {
        Context* ctx = Context::getContext();
        void* glCtx = createGLContext();
        ctx->collector->limit = 10;
        ctx->collector->prepare();
        float delta = 1.0f / frameRate;
        float duration = ctx->mainComposition->getDuration();
        profile(InitOpenGL) {
            initGL();
        }
        GPUFrame blackBackground = GPUFrameCache::alloc(
                nearestPOT(ctx->mainComposition->width),
                nearestPOT(ctx->mainComposition->height));
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
            writeFuc(frame->toCPU());
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
        renderProgress = 100;
    });
}

void CCPlus::render() {
    if(renderMode == FINAL_MODE) {
        std::string outfile = outputPath;
        if(!stringEndsWith(outfile, ".mp4")) {
            outfile = Context::getContext()->getStoragePath("result.mp4");
        }
        VideoEncoder encoder(outfile, frameRate);
        renderAs([&encoder](const Frame& frame) {
            encoder.appendFrame(frame);
        });
        waitRender();
        encoder.finish();
    } else {
        int fn = 0;
        renderAs([&fn](const Frame& frame) {
            char buf[20];
            sprintf(buf, "%07d.zim", fn++);
            frame.write(Context::getContext()->getStoragePath(buf));
        });
        waitRender();
    }
    renderProgress = 100;
}

int CCPlus::getRenderProgress() {
    return renderProgress;
}

void CCPlus::waitRender() {
    if (!render_thread) return;
    pthread_join(render_thread, 0);
}

int CCPlus::numberOfFrames() {
    Context* ctx = Context::getContext();
    float d = 1.0 / frameRate;
    return ctx->mainComposition->getDuration() / d;
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

    setOutputPath(outputPath);
    for (int i = 0; i < size; i++) {
        initContext(generatePath(dir, "COVER" + toString(i) + ".tml"));
        //, outputPath, 18);
        render();
        waitRender();
        releaseContext();
        Frame f;
        f.read(generatePath(outputPath, "0000000.zim"));
        imwrite(generatePath(outputPath, toString(i) + ".jpg"), f.image);
    }
}
