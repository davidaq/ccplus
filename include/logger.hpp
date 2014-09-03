#pragma once

#include <cstdio>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

#include <pthread.h>

#define L() \
    log(logDEBUG)

#define log(level) if (level > logLevel) ; \
    else CCPlus::Logger(level, __LINE__, __FILE__)
 
namespace CCPlus {
    class Logger;
    enum LogLevel {
        logFATAL, logERROR, logWARN, logINFO, logDEBUG, logDEBUG1
    };
}

class CCPlus::Logger {

public:
    Logger(CCPlus::LogLevel _level = CCPlus::logINFO, 
            int lineNumber = -1, 
            const std::string& file = "");
    ~Logger();

    template <typename T>
    Logger& operator<<(T const & value) {
        _buffer << value;
        return *this;
    }
private: 
    CCPlus::LogLevel level;
    std::ostringstream _buffer;
    std::ostream* out = &std::cout;
    int lineNumber;
    std::string file;

    pthread_mutex_t printLock;

    std::vector<std::string> cols = {
        "\x1b[31m", // Red for Fatal
        "\x1b[31m", // Red for Error
        "\x1b[33m", // Yellow for Warning
        "\x1b[34m", // Blue for Info
        "\x1b[36m", // Cyan for Debug
        "\x1b[36m", // Cyan for Debug
        "\x1b[36m", // Cyan for Debug
        "\x1b[36m", // Cyan for Debug
        "\x1b[36m", // Cyan for Debug
    };
};

extern CCPlus::LogLevel logLevel;
