#include "mat-cache.hpp"
#include "logger.hpp"

using namespace CCPlus;

pthread_mutex_t MatCache::cacheLock;
std::map<std::string, cv::Mat> MatCache::cachePool;
std::list<std::string> MatCache::cacheQueue;

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
    cacheQueue.clear();
    pthread_mutex_unlock(&cacheLock);
}

cv::Mat MatCache::get(const std::string& hashname, std::function<cv::Mat()> logic) {
    init();
    pthread_mutex_lock(&cacheLock);
    cv::Mat ret;
    if(cachePool.count(hashname)) {
        ret = cachePool[hashname];
        cacheQueue.remove(hashname);
        cacheQueue.push_back(hashname);
    } else {
        pthread_mutex_unlock(&cacheLock);
        ret = logic();
        pthread_mutex_lock(&cacheLock);
        cachePool[hashname] = ret;
        cacheQueue.push_back(hashname);
        while(cacheQueue.size() > 20) {
            cachePool.erase(cacheQueue.front());
            cacheQueue.pop_front();
            L() << "Erase cache";
        }
    }
    pthread_mutex_unlock(&cacheLock);
    return ret;
}

void HashFactory::appendHashSum() {
    if(hashSumCounter) {
        static const char* numMap = "0123456789qwerytuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-+";
        while(hashSum) {
            hashStr += numMap[63 & hashSum];
            hashSum >>= 6;
        }
        hashStr += hashSum;
    }
    hashSum = hashSumCounter = 0;
}

HashFactory& HashFactory::operator << (std::string str) {
    appendHashSum();
    hashStr += str + "._.";
    return *this;
}

HashFactory& HashFactory::operator << (int num) {
    if(hashSumCounter > 4)
        appendHashSum();
    unsigned int head = (hashSum & 0xff000000) >> 24;
    hashSum = (hashSum << 8) + num + head;
    hashSumCounter++;
    return *this;
}

HashFactory& HashFactory::operator << (float num) {
    int i = 0;
    for(i = 0; i < 10 && num < 10000; i++)
        num *= 10;
    this->operator << ((int)num);
    this->operator << (i);
    return *this;
}

const std::string& HashFactory::str() {
    appendHashSum();
    return hashStr;
}
