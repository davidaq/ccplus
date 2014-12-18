/***********************************************************************************
 * CCPlus Player Helper
 *
 * These functions provide utilities for player developer.
 * These utilities are NOT thread-safe
 ***********************************************************************************/

#pragma once

namespace CCPlus {
    // @ Play from start
    void rewind();
    // Main function
    // Will check framebuffer and upload needed frames to @imageaction and @audioaction
    void play(const std::string& zimDir, int fps);
    //void play(const std::string& zimDir, std::function<void()> imageAction = nullptr, std::function<void()> audioAction = nullptr);
    // @ Stop scanning and playing
    void stop();

    typedef void (*PlayerInterface)(
        float currentTime,
        const char* imageData, int imageWidth, int imageHeight, 
        const char* audioData, int audioLen,
        float bgmVolume
    );
    void attachPlayerInterface(PlayerInterface);
}
