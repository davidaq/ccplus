#pragma once
#include <string>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>
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

static inline int getImageRotation(const std::string& jpgpath) {
    FILE* f = fopen(jpgpath.c_str(), "rb");   
    int ret = -1;
    if (f == NULL) return ret; 
    char tmp[64];

    auto consume = [](FILE* f, int n) {
        fseek(f, n, SEEK_CUR);
    };

    auto nread = [](FILE* f, char* tmp, int origin, int offset) {
        for (int i = 0; i < offset; i++)  
            if (fscanf(f, "%c", &tmp[origin + i]) != 1) 
                throw std::ios_base::failure("Unrecgonized file format");
    };

    auto bytesToInt = [](const char* s, int n) {
        int ret = 0;
        for (int i = 0; i < n; i++) 
            ret = ret * 256 + s[i];
        return ret;
    };

    std::map<int, int> formatBytes = {
        {1, 1}, {2, 1}, {3, 2}, {4, 4}, {5, 8}, {6, 1},
        {7, 1}, {8, 2}, {9, 4}, {10, 8}, {11, 4}, {12, 8}
    };

    std::map<int, int> retTable = {
        {-1, 0}, {1, 0}, {3, 180}, {8, 270}, {6, 90}
    };

    enum State {
        START = 0, EXIF, TIFF, IFD, ORIENTATION, DONE
    };

    State state = START;
    
    // Assume 2 bytes reading is OK 
    while (state != DONE && (fscanf(f, "%c%c", &tmp[0], &tmp[1]) > 1)) {
        if (state == START) {
            // Found ffe1 -> app1 marker !!
            if ((unsigned char) tmp[0] == 0xff && (unsigned char) tmp[1] == 0xe1) {
                state = EXIF;
                continue;
            }
        } else if (state == EXIF) {
            if (!strncmp(tmp, "Ex", 2)) {
                nread(f, tmp, 2, 2);
                if (!strncmp(tmp, "Exif", 4)) {
                    state = TIFF;
                    // Eat empty bytes
                    consume(f, 2);
                    continue;
                }
            }
        } else if (state == TIFF) {
            // GOT TFIF reader
            if (!strncmp(tmp, "MM", 2)) {
                state = IFD;

                // EAT tag mask
                consume(f, 2);

                nread(f, tmp, 0, 4);

                int skip = bytesToInt(tmp, 4) - 8;
                consume(f, skip);
                continue;
            } else if (!strncmp(tmp, "II", 2)) {
                // DAMN the Intel
                return -1;
            }

        } else if (state == IFD) {
            int nifd = bytesToInt(tmp, 2);
            for (int i = 0; i < nifd; i++) {
                // Read tag number
                nread(f, tmp, 0, 2);
                // Not rotation
                if (bytesToInt(tmp, 2) == 0x0112) 
                    state = ORIENTATION;   

                // Read format
                nread(f, tmp, 0, 2);
                // Must be a short -> 3
                int format = bytesToInt(tmp, 2);
                int sz = formatBytes[format];

                // Read component number -> must be 0x00000001
                nread(f, tmp, 0, 4);
                
                int ncomp = bytesToInt(tmp, 4);

                // If total bytes more than 4, then skip
                int totalb = sz * ncomp;
                if (totalb > 4) {
                    consume(f, 4);
                    continue;
                }

                // Read data
                for (int j = 0; j < ncomp; j++)
                    nread(f, tmp, 0, sz);
                
                if (state == ORIENTATION) {
                    ret = bytesToInt(tmp, 2);
                    state = DONE;
                    break;
                }
            }
        }
        fseek(f, -1, SEEK_CUR);
    }

    fclose(f);
    // CW
    return retTable[ret];
}

static inline void stringToLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}
