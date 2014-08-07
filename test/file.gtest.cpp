#include "gtest/gtest.h"
#include "file-manager.hpp"
#include "file.hpp"
#include "logger.hpp"
using namespace CCPlus;

TEST(FileUtil, InMemoryReadWrite) {
    FileManager fileManager = *FileManager::getInstance();
    
    File* f = fileManager.open("tmp/yo", "w", true);
    
    int tmp[] = {1, 2, 3, 4};
    f->write(tmp, sizeof(int), 4); 
    f->close();

    f = fileManager.open("tmp/yo", "r", true);
    int* ret = (int*)f->readAll();
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(i + 1, ret[i]);
}

TEST(FileUtil, ReadWrite) {
    FileManager fileManager = *FileManager::getInstance();
    
    File* f = fileManager.open("tmp/yo", "w");
    
    int tmp[] = {1, 2, 3, 4};
    f->write(tmp, sizeof(int), 4); 
    f->close();

    f = fileManager.open("tmp/yo", "r");
    int* ret = (int*)f->readAll();
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(i + 1, ret[i]);
    f->close();
}
