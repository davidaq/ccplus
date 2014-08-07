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

protected:
    FileManager();
    std::map<std::string, File*> storage;
    pthread_mutex_t storageLock;
};
