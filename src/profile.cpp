#include "profile.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <map>
#include <pthread.h>

using namespace CCPlus;

std::map<std::string, double>* pmap = 0;
pthread_mutex_t pmaplock;

void pinit() {
    if(pmap)
        return;
    pmap = new std::map<std::string ,double>();
    pthread_mutex_init(&pmaplock, 0);
}

Profiler::Profiler(const char* _name):
    name(_name)
{
    startTime = getSystemTime();
}

Profiler::Profiler(const std::string& _name):
    name(_name)
{
    startTime = getSystemTime();
}

Profiler::~Profiler() {
    pinit();
    double total = getSystemTime() - startTime;
    pthread_mutex_lock(&pmaplock);
    double prev = pmap->count(name) ? (*pmap)[name] : 0;
    (*pmap)[name] = prev + total;
    pthread_mutex_unlock(&pmaplock);
}

void Profiler::flush() {
    pinit();
    pthread_mutex_lock(&pmaplock);
    std::map<std::string, double>::iterator it = pmap->begin();
    char tbuff[20];
    log(logINFO) << "PROFILE: ";
    for(; it != pmap->end(); it++) {
        sprintf(tbuff, "%fs", it->second);
        log(logINFO) << "   " << it->first << ": " << tbuff;
    }
    pmap->clear();
    pthread_mutex_unlock(&pmaplock);
}
