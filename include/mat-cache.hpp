#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <pthread.h>

namespace CCPlus {
    class MatCache;
    class HashFactory;
    struct CacheItem;
};

class CCPlus::MatCache {
public:
    static cv::Mat get(const std::string& hashname, std::function<cv::Mat()> logic);
    static void clear();
private:
    static void init();
    static std::map<std::string, cv::Mat> cachePool;
    static std::list<std::string> cacheQueue;
    static pthread_mutex_t cacheLock;
};

class CCPlus::HashFactory {
public:
    HashFactory& operator << (std::string str);
    HashFactory& operator << (int num);
    HashFactory& operator << (float num);
    const std::string& str();
private:
    void appendHashSum();
    std::string hashStr;
    unsigned int hashSum = 0;
    int hashSumCounter = 0;
};

