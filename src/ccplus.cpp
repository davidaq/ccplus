#include "global.hpp"
#include "ccplus.hpp"
#include "context.hpp"

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
    // TODO implement
}

void CCPlus::encode() {
    // TODO implement
}

int CCPlus::numberOfFrames() {
    // TODO implement
    return 0;
}

