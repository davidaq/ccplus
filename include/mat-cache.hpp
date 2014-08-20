#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <map>
#include <functional>
#include <pthread.h>

namespace CCPlus {
    class MatCache;
};

class CCPlus::MatCache {
public:
    static cv::Mat get(const std::string& hashname, std::function<cv::Mat()> logic);
    static void clear();
private:
    static void init();
    static std::map<std::string, cv::Mat> cachePool;
    static pthread_mutex_t cacheLock;
};
