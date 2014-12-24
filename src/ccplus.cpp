#include "global.hpp"
#include "ccplus.hpp"
#include "ccplus-base.hpp"
#include "parallel-executor.hpp"


using namespace CCPlus;

// The life-cycle of these two object are managed by rendering thread
Lock renderLock;
RenderTarget activeTarget(0);
RenderTarget pendingTarget(0);

RenderTarget::RenderTarget(int index) {
    this->index = index;
    if(index < 0) {
        static int counter = 1;
        this->index = counter++;
    }
}

RenderTarget::~RenderTarget() {
    stop();
}

bool RenderTarget::isActive() const {
    renderLock.lock();
    bool ret = (index > 0 && index == activeTarget.index);
    renderLock.unlock();
    return ret;
}

bool RenderTarget::isPending() const {
    renderLock.lock();
    bool ret = (index > 0 && index == pendingTarget.index);
    renderLock.unlock();
    return ret;
}

bool RenderTarget::isProcessing() const {
    bool ret = true;
    if(index > 0) {
        renderLock.lock();
        ret = index == pendingTarget.index || index == activeTarget.index;
        renderLock.unlock();
    }
    return ret;
}

void RenderTarget::stop() {
    if (isActive()) {
        CCPlus::stop();
        return;
    }
    if (isPending()) {
        renderLock.lock();
        pendingTarget = 0;
        renderLock.unlock();
        return;
    }
}

void RenderTarget::waitFinish() {
    while (isProcessing()) {
        usleep(100000);
    }
}

int RenderTarget::getProgress() const {
    if(isActive()) {
        return renderProgress;
    }
    return 0;
}

void RenderTarget::invalidate() {
    index = 0;
}

RenderTarget::operator bool() {
    return index > 0;
}

RenderTarget& RenderTarget::operator = (int nindex) {
    index = nindex;
    return *this;
}

void CCPlus::go(const std::string& path, int fps) {
    RenderTarget target;
    target.inputPath = path;
    target.fps = fps;
    go(target);
    target.waitFinish();
}

void CCPlus::go(const RenderTarget& target) {
    if (target.isProcessing()) {
        return;
    }
    activeTarget.stop();
    renderLock.lock();
    pendingTarget = target;
    renderLock.unlock();
    static pthread_t render_thread = 0;
    if (!render_thread) {
        /*************************************
         * Only this thread will be able to modify @activeTarget
         ************************************/
        render_thread = ParallelExecutor::runInNewThread([] () {
            while (true) {
                renderLock.lock();
                if (!pendingTarget || activeTarget) {
                    renderLock.unlock();
                    break;
                }
                activeTarget = pendingTarget;
                pendingTarget = 0;

                renderProgress = 0;
                continueRunning = true;
                setOutputPath(activeTarget.outputPath);
                setFrameRate(activeTarget.fps);
                int mode = activeTarget.mode;
                setRenderMode(mode);
                std::string path = activeTarget.inputPath;
                renderLock.unlock();

                if(!stringEndsWith(path, ".tml")) {
                    path = generateTML(path, mode==PREVIEW_MODE);
                }
                profile(InitContext) {
                    CCPlus::initContext(path);
                }
                // Keep GL context in a seprated thread context
                pthread_t glThread = ParallelExecutor::runInNewThread([] () {
                    CCPlus::render();
                });
                pthread_join(glThread, NULL);
                releaseContext();

                renderLock.lock();
                continueRunning = false;
                activeTarget = 0;
                renderLock.unlock();
            }
            render_thread = 0;
        });
    }
}
