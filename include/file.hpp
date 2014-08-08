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
    // Might be unsafe
    void readAll(void* ret);

    void read(void* ret, int len);
    void write(const void* buffer, std::size_t size, std::size_t count=1);

    FILE* getFile();
    void clear();

    int getSize() const;

protected:
    std::vector<unsigned char>* vec = nullptr;
    FILE* file = nullptr;
    bool inMemory;
};
