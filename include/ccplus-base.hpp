#pragma once

namespace CCPlus {
    void stop();
    void initContext(const std::string& tmlPath, const std::string& footageDir="");
    void releaseContext();

    void render();

    extern int renderProgress;
    extern bool continueRunning;
};
