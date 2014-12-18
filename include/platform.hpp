#pragma once

namespace CCPlus {
    // OpenGL Context
    void* createGLContext();
    void destroyGLContext(void*);

    // delete file
    void removeFile(const std::string& fileName, bool removeDir=false);
}
