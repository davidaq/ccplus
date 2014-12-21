#pragma once

namespace CCPlus {
    void stop();
    void initContext(const std::string& tmlPath);
    void releaseContext();

    void render();

    extern int renderProgress;
    extern bool continueRunning;
};
