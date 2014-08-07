#include "file-manager.hpp"
#include "logger.hpp"
#include "file.hpp"

using namespace CCPlus;

FileManager::FileManager() {
    pthread_mutex_init(&storageLock, 0);
}

FileManager::~FileManager() {
    pthread_mutex_lock(&storageLock);
    for (auto& kv : storage) 
        delete kv.second;
    pthread_mutex_unlock(&storageLock);
}

FileManager* FileManager::getInstance() {
    static FileManager fm;       
    return &fm;
}

File* FileManager::open(const std::string& fn, const std::string& mode, bool inMemory) {
    File* ret = nullptr;
    if (!inMemory) {
        ret = new File(fn, mode, inMemory);
        if (ret->getFile() == NULL)
            return this->open(fn, mode, !inMemory);
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
