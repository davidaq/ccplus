#pragma once


#ifndef GLSLES
#if defined(__ANDROID__) || defined(__IOS__)
#define GLSLES
#endif
#endif


// Runtime Dynamic Configurations

namespace CCPlus {
    extern std::string assetsPath;
    extern std::string outputPath;
    extern int audioSampleRate;
    extern int frameRate;
    extern int renderMode;
    extern int renderFlag;

    const static int FORCE_POT = 1, LARGE_MEM = 1 << 1, JSON_BEUTIFY = 1 << 2;

    extern int collectorThreadsNumber;
    extern float collectorTimeInterval;
    extern int collectorWindowSize;
};

