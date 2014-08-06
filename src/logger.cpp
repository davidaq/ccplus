#include "logger.hpp"
#include "utils.hpp"

#include <ctime>

using namespace CCPlus;

CCPlus::LogLevel logLevel = CCPlus::logDEBUG;

Logger::Logger(LogLevel _level, int _ln, const std::string& _file) 
: level(_level), lineNumber(_ln), file(_file) {
    if (level < logWARN)
        out = &std::cerr;
    _buffer << cols[level];
    _buffer << "[" << getFormatedTime("%c %Z") << "] ";

    if (level >= logDEBUG && lineNumber != -1 && file != "") {
        _buffer << file << ":" << lineNumber << " ";
    }
}

Logger::~Logger() {
    _buffer << "\x1b[0m" << std::endl;
    (*out) << _buffer.str();

    if (level == logFATAL)
        exit(-1);
}
