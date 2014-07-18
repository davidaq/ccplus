#pragma once
#include <string>
#include <cstdio>
#include <cstring>
#include <map>
#include <algorithm>
#include <string>

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

template<class T> static inline 
T parseString(const std::string& str) {
    T value;
    std::istringstream iss(str);
    iss >> value;
    return value;
}

template<bool> static inline 
bool parseString(const std::string& str) {
    bool value;
    std::istringstream iss(str);
    iss >> std::boolalpha >> value;
    return value;
}

template<class T> static inline 
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

static inline std::string toString(const bool& value) {
    std::ostringstream oss;
    oss << std::boolalpha << value;
    return oss.str();
}

static inline std::string ltrim(std::string str) {
    return str.erase(0, str.find_first_not_of(" /t/n/r"));
}

static inline std::string rtrim(std::string str) {
    return str.erase(str.find_last_not_of(" /t/n/r") + 1);
}

static inline std::string trim(std::string str) {
    return rtrim(ltrim(str));
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
        for (int i = 0; i < offset; i++)  fscanf(f, "%c", &tmp[origin + i]);
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
        {-1, -1}, {1, 0}, {3, 180}, {8, 270}, {6, 90}
    };

    enum State {
        START = 0, EXIF, TIFF, IFD, ORIENTATION, DONE
    };

    State state = START;
    
    // Assume 2 bytes reading is OK
    while (state != DONE && (fscanf(f, "%c%c", &tmp[0], &tmp[1]) > 0)) {
        if (state == START) {
            // Found ffe1 -> app1 marker !!
            if ((unsigned char) tmp[0] == 0xff && (unsigned char) tmp[1] == 0xe1) {
                state = EXIF;
                continue;
            }
        } else if (state == EXIF) {
            if (!strncmp(tmp, "Ex", 2)) {
                fscanf(f, "%c%c", &tmp[2], &tmp[3]);
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
