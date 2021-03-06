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
#include "platform.hpp"

namespace CCPlus {
    cv::Mat readAsset(const char* name);
    int getImageRotation(const uint8_t* data, uint32_t len); 
    bool hasAudio(const std::string& uri, float start, float duration);
    bool checkGLError();
    bool checkGLFramebuffer();
    bool isGLFramebufferComplete();
}

static inline double getSystemTime() { 
    // Might not work at multicore situation
    struct timeval now;
    gettimeofday (&now, NULL);
    return now.tv_usec / 1000000.0 + now.tv_sec;
}

inline bool file_exists(const std::string& s) {
    if (FILE* file = fopen(s.c_str(), "r")) {
        fclose(file);
        return true;
    } 
    return false;
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
    size_t pos = content.rfind(suffix);
    if(pos == std::string::npos)
        return false;
    return pos == (content.length() - suffix.length());
}

static inline bool stringStartsWith(std::string content, std::string prefix) {
    return content.find(prefix) == 0;
}

static inline std::string toLower(const std::string& s) {
    std::string data = s;
    const static auto tol = [](char c) {
        if (c >= 'A' && c <= 'Z')
            return char(c - 'Z' + 'z');
        return c;
    };
    std::transform(data.begin(), data.end(), data.begin(), tol);
    return data;
}

static inline std::string toUpper(const std::string& s) {
    std::string data = s;
    const static auto tol = [](char c) {
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

// Used to convert logical unix like path to system specific path
static inline std::string generatePath(const std::string& dir, const std::string& fn) {
    std::string ret;
    if (dir == "" || fn[0] == '/') {
        ret = fn;
    } else {
        ret = dir + "/" + fn;
    }
    std::string::iterator r = ret.begin(), w = ret.begin();
    int sz = 0;
    for(; r != ret.end(); r++) {
        if(*r == '/') {
            *(w++) = getSeperator();
            sz++;
            while(*r == '/' && r != ret.end())
                r++;
            if(r == ret.end())
                break;
        }
        *(w++) = *r;
        sz++;
    }
    ret.resize(sz);
#ifdef _WIN32
    if(ret[0] == '\\' && ret[2] == ':' && ret[3] == '\\') {
        // convert \C:\Program Files to C:\Program Files
        return ret.substr(1);
    }
#endif
    return ret;
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

static inline void utf8toWStr(std::wstring& dest, const std::string& src){
    dest.clear();
    wchar_t w = 0;
    int bytes = 0;
    wchar_t err = L' ';
    for (size_t i = 0; i < src.size(); i++){
        unsigned char c = (unsigned char)src[i];
        if (c <= 0x7f){//first byte
            if (bytes){
                dest.push_back(err);
                bytes = 0;
            }
            dest.push_back((wchar_t)c);
        }
        else if (c <= 0xbf){//second/third/etc byte
            if (bytes){
                w = ((w << 6)|(c & 0x3f));
                bytes--;
                if (bytes == 0)
                    dest.push_back(w);
            }
            else
                dest.push_back(err);
        }
        else if (c <= 0xdf){//2byte sequence start
            bytes = 1;
            w = c & 0x1f;
        }
        else if (c <= 0xef){//3byte sequence start
            bytes = 2;
            w = c & 0x0f;
        }
        else if (c <= 0xf7){//3byte sequence start
            bytes = 3;
            w = c & 0x07;
        }
        else{
            dest.push_back(err);
            bytes = 0;
        }
    }
    if (bytes)
        dest.push_back(err);
}

static inline void mat3to4(cv::Mat& org) {
    if(org.channels() == 4)
        return;
    cv::Mat newimg = cv::Mat(org.rows, org.cols, CV_8UC4, {0, 0, 0, 255});
    if(org.channels() < 1 || org.channels() > 3) {
        org = newimg;
        return;
    }
    static const int from_to[][6] {
        {0, 0, 0, 1, 0, 2}, // 1 channel
        {0, 0, 0, 1, 0, 2}, // 2 channel
        {0, 0, 1, 1, 2, 2}, // 3 channel
    };
    mixChannels(&org, 1, &newimg, 1, from_to[org.channels() - 1], 3);
    org = newimg;
}

static inline std::string readTextAsset(const std::string& path) {
    cv::Mat raw = CCPlus::readAsset(path.c_str());
    const char* ptr = (const char*)raw.data;
    return std::string(ptr, ptr + raw.total());
}

// get nearest power of two
static inline int nearestPOT(const int n, bool larger=false) {
    if(!(CCPlus::renderFlag & CCPlus::FORCE_POT)) {
        if(n < 2048)
            return n;
        return 2048;
    }
    const static int pots[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    const static int potsN = 8;
    int pd = 0xffff;
    int ret = 512;
    for(int i = 0; i < potsN; i++) {
        int d = pots[i] - n;
        if(d < 0) d = -d;
        if(d < pd) {
            ret = pots[i];
            pd = d;
        }
    }
    if(larger && ret < n)
        ret *= 2;
    return ret;
}

static inline float Fabs(float v) {
    if(v > 0)
        return v;
    return -v;
}

