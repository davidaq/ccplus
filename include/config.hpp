#pragma once

// Constant Compile-time Configuration

const static bool JSON_BEUTIFY        = false;
const static bool USE_POT_TEXTURE     = true;

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
    extern int collectorThreadsNumber;
    extern int collectorTimeInterval;
    extern int frameRate;
    extern int renderMode;
};

