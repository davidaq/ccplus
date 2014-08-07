#include "file.hpp"
#include <string>
#include "logger.hpp"

using namespace CCPlus;

File::File(const std::string& fn, const std::string& mode, bool _inMemory) {
    this->inMemory = _inMemory;
    if (inMemory) {
        vec = new std::vector<unsigned char>();
    } else {
        file = fopen(fn.c_str(), mode.c_str());
    }
}

File::~File() {
    if (file) {
        fclose(file);
        delete file;
    }

    if (vec)
        delete vec;

    if (fileContent)
        delete fileContent;
    vec = nullptr;
    file = nullptr;
}

void File::close() {
    if (file) {
        fclose(file);
        file = nullptr;
        if (fileContent) {
            delete fileContent;
            fileContent = nullptr;
        }
        delete this;
    }
}

const unsigned char* File::readAll() {
    if (inMemory) {
        unsigned char* tmp = new unsigned char[vec->size()];
        memcpy(tmp, &vec->front(), vec->size());
        return tmp;
    } else {
        fseek(file, 0, SEEK_END);       
        size_t len = ftell(file);
        fseek(file, 0, SEEK_SET);       
        fileContent = new unsigned char[len];
        fread(fileContent, sizeof(unsigned char), len, file);       
        return this->fileContent;
    }
}

void File::write(const void* buffer, std::size_t size, std::size_t count) {
    if (inMemory) {
        for (int i = 0; i < count * size; i++) {
            vec->push_back(((unsigned char*) buffer)[i]);
        }
    } else {
        fwrite(buffer, size, count, file);
    }
}
