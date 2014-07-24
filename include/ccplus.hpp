#pragma once 

namespace CCPlus {
    void* initContext(const char* tmlPath, const char* storagePath, int fps);
    void go(
            const std::string&, const std::string&,
            float start = 0, float length = 5,
            int fps= 18);
    void renderImage(
            const std::string&, const std::string&,
            float start = 0, float length = 5,
            int fps= 18);
}
