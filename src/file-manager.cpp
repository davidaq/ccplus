#include "file-manager.hpp"
#include "logger.hpp"
#include "file.hpp"

using namespace CCPlus;

FileManager::FileManager() {
    pthread_mutex_init(&storageLock, 0);
    pthread_mutex_init(&linksLock, 0);
}

FileManager::~FileManager() {
    clear();
}

FileManager* FileManager::getInstance() {
    static FileManager fm;       
    return &fm;
}

void FileManager::clear() {
    pthread_mutex_lock(&storageLock);
    for (auto& kv : storage) 
        delete kv.second;
    storage.clear();
    pthread_mutex_unlock(&storageLock);
    pthread_mutex_lock(&linksLock);
    links.clear();
    pthread_mutex_unlock(&linksLock);
}

void FileManager::addLink(const std::string& src, const std::string& linkTo) {
    pthread_mutex_lock(&storageLock);
    if (links.count(src)) {
        log(logFATAL) << "Link already existed for: " << src;
    }
    if (links.count(linkTo))
        links[src] = links[linkTo];
    else
        links[src] = linkTo;
    pthread_mutex_unlock(&storageLock);
}

File* FileManager::open(const std::string& fn, const std::string& mode, bool inMemory) {
    if (links.count(fn))
        return this->open(links[fn], mode, inMemory);
    File* ret = nullptr;
    if (!inMemory) {
        ret = new File(fn, mode, inMemory);
        if (ret->getFile() == NULL) {
            ret->close();
            return this->open(fn, mode, !inMemory);
        }
        return ret;
    }
    if (storage.count(fn) == 1) {
        pthread_mutex_lock(&storageLock);
        // TODO: try to avoid exception. Potential dead-lock here
        try {
            ret = storage.at(fn);
        } catch(...) {
            log(logERROR) << "Concurrency problem happened!";
            log(logFATAL) << "Couldn't find file: " << fn;      
        } 
        pthread_mutex_unlock(&storageLock);
        if (mode[0] == 'w') ret->clear();
    } else {
        // TODO find a better way to handle error
        if (mode[0] != 'a' && mode[0] != 'w') {
            log(logFATAL) << "Couldn't find file: " << fn;
        }
        ret = new File(fn, mode, inMemory);
        pthread_mutex_lock(&storageLock);
        //storage.insert(std::make_pair(fn, tmp));
        storage[fn] = ret;
        pthread_mutex_unlock(&storageLock);
    }
    return ret;
}
