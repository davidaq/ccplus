#pragma once

#include <string>

#ifndef RELEASE
#define profile(name) for(CCPlus::Profiler PROFILER_ ## name(#name);!PROFILER_ ## name .passed;PROFILER_ ## name .passed = true)
#define profileFlush CCPlus::Profiler::flush()
#else
#define profile(X)
#define profileFlush
#endif

namespace CCPlus {
    class Profiler;
};

class CCPlus::Profiler {
public:
    explicit Profiler(const char* name);
    ~Profiler();
    static void flush();
    bool passed = false;
private:
    double startTime;
    std::string name;
};

