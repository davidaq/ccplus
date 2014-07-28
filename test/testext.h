#pragma once
#include <string>
#include "stdlib.h"


#define EXPECT_FILE_HASH_EQ(HashString, FileName) EXPECT_STREQ(file_hash( #FileName ).c_str(), #HashString )

static inline std::string file_hash(const char* filename) {
    char buff[300];
    sprintf(buff, "./scripts/run filehash.py './%s' > .__hash", filename);
    system(buff);
    FILE* fp = fopen(".__hash", "r");
    fscanf(fp, "%s", buff);
    return buff;
    system("rm -f .__hash");
}


