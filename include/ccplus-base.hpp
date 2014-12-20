#pragma once

namespace CCPlus {
    void stop();
    void initContext(const std::string& tmlPath);
    void releaseContext();
    int getProgress();

    void render();

    extern bool continueRunning;
};
