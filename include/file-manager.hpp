#pragma once

#include <semaphore.h>
#include <map>
#include <vector>

namespace CCPlus {
    class FileManager;
    class File;
};

/*
 * A thread-safe file abstraction
 */
class CCPlus::FileManager {
public:
    ~FileManager();
    static FileManager* getInstance();
    CCPlus::File* open(const std::string& filename, 
            const std::string& mode, 
            bool inMemory = false);
    void addLink(const std::string& src, 
            const std::string& linkTo);

    void clear();

protected:
    FileManager();
    std::map<std::string, File*> storage;
    std::map<std::string, std::string> links;
    pthread_mutex_t linksLock;
    pthread_mutex_t storageLock;
};
