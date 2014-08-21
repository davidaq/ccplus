#pragma once
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <sys/time.h>
#include <ctime>

#include <iomanip>

#include "logger.hpp"

static inline double getSystemTime() { 
    // Might not work at multicore situation
    struct timeval now;
    gettimeofday (&now, NULL);
    long long tmp = now.tv_usec + (uint64_t)now.tv_sec * 1000000;
    return tmp / 1000000.0;
}

// Thread-safe time
static inline std::string getFormatedTime(const std::string& fmt, int n = 256) {
    char str[n];
    std::tm tm_snapshot;
    std::time_t nowtime = std::time(nullptr);
    // localtime_r is thread safe
    localtime_r(&nowtime, &tm_snapshot);
    int ret = std::strftime(str, n, "%c %Z", &tm_snapshot);
    if (ret == 0) return "";
    return std::string(str);
}

static inline bool stringEndsWith(std::string content, std::string suffix) {
    return content.rfind(suffix) == (content.length() - suffix.length());
}

static inline bool stringStartsWith(std::string content, std::string prefix) {
    return content.find(prefix) == 0;
}

static inline std::string toLower(const std::string& s) {
    std::string data = s;
    auto tol = [](char c) {
        if (c >= 'A' && c <= 'Z')
            return char(c - 'Z' + 'z');
        return c;
    };
    std::transform(data.begin(), data.end(), data.begin(), tol);
    return data;
}

static inline std::string toUpper(const std::string& s) {
    std::string data = s;
    auto tol = [](char c) {
        if (c >= 'a' && c <= 'z')
            return char(c + 'Z' - 'z');
        return c;
    };
    std::transform(data.begin(), data.end(), data.begin(), tol);
    return data;
}

static inline char getSeperator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

template <typename T>
static inline std::string toString(T number) {
    std::ostringstream ss;
    ss << number;
    return ss.str();
}

template <typename T>
static inline T between(T t, T mn, T mx) {
    return std::max(mn, std::min(mx, t));
}

static inline std::string dirName(const std::string& path) {
    return path.substr(
            0,
            std::find(path.rbegin(), 
                path.rend(), 
                getSeperator()).base() - path.begin());
}

static inline std::string generatePath(const std::string& dir, const std::string& fn) {
    if (dir == "") return fn;
    //if (!boost::filesystem::exists(dir))
    //    boost::filesystem::create_directory(dir);
    if (dir[dir.length() - 1] != getSeperator())
        return dir + getSeperator() + fn;
    return dir + fn;
}

static inline std::string slurp(const std::string& file) {
    std::ifstream fstream(file, std::ios::in);
    if (!fstream.is_open()) {
        log(CCPlus::logFATAL) << "Couldn't slurp file: " << file;
    }
    std::stringstream buffer;
    buffer << fstream.rdbuf();
    std::string ret = buffer.str();
    fstream.close();
    return ret;
}

static inline void spit(const std::string& file, const std::string& content) {
    std::ofstream fstream;
    fstream.open(file);
    if (!fstream.is_open()) {
        log(CCPlus::logFATAL) << "Couldn't open write file " << file;
    }
    fstream << content;
    fstream.close();
}

static inline void stringToLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

int getImageRotation(const std::string& jpgpath); 
