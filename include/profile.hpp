#pragma once

#include <string>

#ifndef RELEASE
#define profile(name) for(CCPlus::Profiler PROFILER_ ## name(#name);!PROFILER_ ## name .passed;PROFILER_ ## name .passed = true)
#define profileFlush CCPlus::Profiler::flush()
#define profileBegin(name) \
    CCPlus::Profiler* __profiler ## name = new CCPlus::Profiler(#name);

#define profileEnd(name) \
    delete __profiler ## name;
#else
#define profile(X) if(true)
#define profileFlush
#define profileBegin
#define profileEnd
#endif

namespace CCPlus {
    class Profiler;
};

class CCPlus::Profiler {
public:
    explicit Profiler(const char* name);
    explicit Profiler(const std::string& name);
    ~Profiler();
    static void flush();
    bool passed = false;
private:
    double startTime;
    std::string name;
};

