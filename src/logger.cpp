#include "utils.hpp"
#include <ctime>
#include "logger.hpp"

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <pthread.h>
#endif

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
#ifdef __ANDROID__
    const static char* ANDROID_LOG_TAG[] = {"CCPLUS FATAL","CCPLUS ERROR", "CCPLUS WARN", "CCPLUS INFO",
        "CCPLUS DEBUG", "CCPLUS DEBUG1", "CCPLUS DEBUG2", "CCPLUS DEBUG3"};
    __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG[(int)level], "%s", _buffer.str().c_str());
#else
    static pthread_mutex_t logLock;
    static bool logLockInited = false;
    if(!logLockInited) {
        pthread_mutex_init(&logLock, 0);
        logLockInited = true;
    }
    pthread_mutex_lock(&logLock);
    (*out) << _buffer.str();
    pthread_mutex_unlock(&logLock);
#endif

    if (level == logFATAL)
        exit(-1);
}
