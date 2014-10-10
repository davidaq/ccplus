#include "global.hpp"
#include "ccplus.hpp"
#include "context.hpp"
#include "footage-collector.hpp"

void CCPlus::go(const std::string& tmlPath, const std::string& outputPath, int fps) {
    initContext(tmlPath, outputPath, fps);
    render();
    encode();
    releaseContext();
}

void CCPlus::initContext(const std::string& tmlPath, const std::string& outputPath, int fps) {
    Context::getContext()->begin(tmlPath, outputPath, fps);
}

void CCPlus::releaseContext() {
    Context::getContext()->end();
}

void CCPlus::render() {
    Context::getContext()->collector->prepare();
}

void CCPlus::encode() {
    // TODO implement
}

int CCPlus::numberOfFrames() {
    // TODO implement
    return 0;
}

