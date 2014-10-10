#pragma once
namespace CCPlus {
    void go(const std::string& tmlPath, const std::string& outputPath, int fps);
    void initContext(const std::string& tmlPath, const std::string& outputPath, int fps);
    void releaseContext();
    void render();
    void encode();
    void fillTML(const std::string& json, const std::string& output);
};
