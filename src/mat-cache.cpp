#include "mat-cache.hpp"

using namespace CCPlus;

pthread_mutex_t MatCache::cacheLock;
std::map<std::string, cv::Mat> MatCache::cachePool;

void MatCache::init() {
    static bool inited = false;
    if(inited)
        return;
    inited = true;
    pthread_mutex_init(&cacheLock, 0);
}

void MatCache::clear() {
    init();
    pthread_mutex_lock(&cacheLock);
    cachePool.clear();
    pthread_mutex_unlock(&cacheLock);
}

cv::Mat MatCache::get(const std::string& hashname, std::function<cv::Mat()> logic) {
    init();
    pthread_mutex_lock(&cacheLock);
    cv::Mat ret;
    if(cachePool.count(hashname)) {
        ret = cachePool[hashname];
    } else {
        pthread_mutex_unlock(&cacheLock);
        ret = logic();
        pthread_mutex_lock(&cacheLock);
        cachePool[hashname] = ret;
    }
    pthread_mutex_unlock(&cacheLock);
    return ret;
}
