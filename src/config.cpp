#include "global.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

std::string CCPlus::assetsPath          = "assets";
std::string CCPlus::outputPath          = "tmp";
int CCPlus::audioSampleRate             = 44100;
int CCPlus::frameRate                   = 18;
int CCPlus::renderMode                  = FINAL_MODE;

int CCPlus::renderFlag                  = FORCE_POT;

int CCPlus::collectorThreadsNumber      = 4;
float CCPlus::collectorTimeInterval     = 0.4;
int CCPlus::collectorWindowSize         = 3;


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

