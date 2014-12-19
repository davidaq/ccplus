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
    void play(const char* zimDir, int fps, bool blocking = false);
    // @ Stop scanning and playing
    void stop();

    typedef void (*PlayerInterface)(
        float currentTime,
        const unsigned char* imageData, int imageWidth, int imageHeight, 
        const unsigned char* audioData, int audioLen,
        float bgmVolume
    );
    void attachPlayerInterface(PlayerInterface);

    typedef void (*ProgressInterface)(
        int precent
    );
    void attachProgressInterface(ProgressInterface);
}
