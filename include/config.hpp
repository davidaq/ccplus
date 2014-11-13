#pragma once

#ifndef GLSLES
#if defined(__ANDROID__) || defined(__IOS__)
#define GLSLES
#endif
#endif

namespace CCPlus {
    extern std::string assetsPath;
    extern std::string outputPath;
    extern int audioSampleRate;
    extern int collectorThreadsNumber;
    extern int frameRate;
    extern int renderMode;
}

