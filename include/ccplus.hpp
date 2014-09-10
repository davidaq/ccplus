#pragma once 

#include <string>

namespace CCPlus {
    void* initContext(const char* tmlPath, const char* storagePath, int fps);
    void releaseContext(void* ctxHandle);
    void freeMemory(void* ctxHandle);
    void renderPart(void* ctxHandle, float start = 0, float length = 5);
    void encodeVideo(void* ctxHandle, float start = 0, float length = -1);
    void go(
            const std::string&, const std::string&,
            float start = 0, float length = 5,
            int fps= 18);

    int numberOfZIM(void*);
    const std::string getZIMPath(void*);
    bool finishedFrame(void*, int frame);

    void fillTML(const std::string& jsonPath, const std::string& outputPath = "");
}
