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
    vec = nullptr;
    file = nullptr;
}

void File::close() {
    if (file) {
        fclose(file);
        file = nullptr;
        delete this;
    }
}

void File::readAll(void* ret) {
    this->read(ret, this->getSize());
}

void File::read(void* ret, int len) {
    if (inMemory) {
        memcpy(ret, &vec->front(), len);
    } else {
        fread(ret, sizeof(unsigned char), len, file);       
    }
}

int File::getSize() const {
    int len = 0;
    if (inMemory) {
        len = vec->size();
    } else {
        fseek(file, 0, SEEK_END);
        len = ftell(file);
        fseek(file, 0, SEEK_SET);
    }
    return len;
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

FILE* File::getFile() {
    return file;
}

void File::clear() {
    vec->clear();
}
