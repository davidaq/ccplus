#include "global.hpp"
#include "ccplus.hpp"

using namespace CCPlus;


std::string CCPlus::assetsPath          = "";
std::string CCPlus::outputPath          = "tmp";
int CCPlus::audioSampleRate             = 32000;
int CCPlus::collectorThreadsNumber      = 3;
int CCPlus::frameRate                   = 18;
int CCPlus::renderMode                  = FINAL_MODE;


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

void CCPlus::setOutputPath(const std::string& val) {
    outputPath = val;
}

void CCPlus::setRenderMode(int val) {
    renderMode = val;
}

