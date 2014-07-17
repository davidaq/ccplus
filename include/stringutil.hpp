#pragma once
#include <string>

static inline bool stringEndsWith(std::string content, std::string suffix) {
    return content.rfind(suffix) == (content.length() - suffix.length());
}

static inline bool stringStartsWith(std::string content, std::string prefix) {
    return content.find(prefix) == 0;
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
