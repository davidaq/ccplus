#pragma once

#include <map>
#include <vector>
#include <cstdio>

#include <semaphore.h>

namespace CCPlus {
    class File;
};

class CCPlus::File {
public:
    File(const std::string& filename, const std::string& mode, bool _inMemory = false);
    File(const std::vector<unsigned char>& vec);

    ~File();
    
    void close();
    const unsigned char* readAll();
    void write(const void* buffer, std::size_t size, std::size_t count=1);
protected:
    std::vector<unsigned char>* vec = nullptr;
    unsigned char* fileContent = nullptr;
    FILE* file = nullptr;
    bool inMemory;
};
