/***********************************************************************************
 * CCPlus Player Helper
 *
 * These functions provide utilities for player developer.
 * These utilities are NOT thread-safe
 * You can consider it as a singleton player
 ***********************************************************************************/

#pragma once

namespace CCPlus {
    namespace CCPlay {
        // Main function
        // It will call @stop() first to make sure there is no running thread
        // After that, this function will be non-blocking
        void play(const char* zimDir, bool blocking = false);
        // @ Stop scanning and playing
        // IT IS A BLOCKING FUNCTION
        void stop();
        // Default 2 seconds
        void setBufferDuration(int); 

        typedef void (*PlayerInterface)(
                float currentTime,
                const unsigned char* imageData, int imageWidth, int imageHeight, 
                const unsigned char* audioData, int audioLen,
                float bgmVolume
                );
        void attachPlayerInterface(PlayerInterface);

        typedef void (*ProgressInterface)(int precent);
        void attachProgressInterface(ProgressInterface);
    };
};
