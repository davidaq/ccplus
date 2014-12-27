#include "global.hpp"
#include "ccplus.hpp"

using namespace CCPlus;


std::string CCPlus::assetsPath          = "assets";
std::string CCPlus::outputPath          = "tmp";
int CCPlus::audioSampleRate             = 44100;
int CCPlus::collectorThreadsNumber      = 2;
int CCPlus::frameRate                   = 18;
int CCPlus::renderMode                  = FINAL_MODE;
int CCPlus::collectorTimeInterval       = 2;


void CCPlus::setAudioSampleRate(int val) {
    audioSampleRate = val;
}

void CCPlus::setAssetsPath(const std::string& val) {
    assetsPath = val;
}

void CCPlus::setCollectorThreadsNumber(int val) {
    collectorThreadsNumber = val;
}

void CCPlus::setFrameRate(int val) {
    frameRate = val;
}

int CCPlus::getFrameRate() {
    return frameRate;
}

void CCPlus::setOutputPath(const std::string& val) {
    outputPath = val;
}

void CCPlus::setRenderMode(int val) {
    renderMode = val;
}

