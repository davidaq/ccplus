#pragma once

namespace CCPlus {

    const int AUDIO_SAMPLE_RATE = 32000;

    const int COLLECTOR_THREAD = 2;
}

#ifndef GLSLES
#if defined(__ANDROID__) || defined(__IOS__)
#define GLSLES
#endif
#endif

extern std::string assetPath;
