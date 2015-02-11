/***********************************************************************************
 * CCPlus Render Engine API header
 * This is the one and only header file exposed to higher level application.
 *
 * Every function listed in this header file are APIs to leverage rendering
 * ability of this engine.
 ***********************************************************************************/

#pragma once

namespace CCPlus {

    // @ configuration functions
    void setAudioSampleRate(int);               // defaults to 32000
    void setAssetsPath(const std::string&);            // defaults to ""
    void setCollectorThreadsNumber(int);        // defaults to 2

    void setFrameRate(int);                     // defaults to 18
    int getFrameRate();
    void setOutputPath(const std::string&);     // defaults to "tmp"
    
    const int PREVIEW_MODE = 0;
    const int FINAL_MODE   = 1;
    void setRenderMode(int);                    // defaults to FINAL_MODE

    class RenderTarget {
    public:
        explicit RenderTarget(int index=-1);

        int fps = 18;
        std::string inputPath = "";
        std::string outputPath = "tmp";
        int mode = FINAL_MODE;

        void stop();
        void waitFinish();
        int getProgress() const;
        bool isActive() const;
        bool isPending() const;
        bool isProcessing() const;

        void invalidate();
        operator bool();
        RenderTarget& operator = (int);
    private:
        int index = 0;

    };

    void onPause();
    void onResume();

    // @ easy one shot render shortcut
    // pass thorugh init, render, wait, release procedure 
    void go(const RenderTarget&);
    // @ Deprecated function. This function is BLOCKING and it's meant be be used as testing
    void go(const std::string&, int fps=15);
    
    // @ Generate a timeline file using a dynamical template, use the half size hint to
    // generate a smaller resolution version of the timeline, which is faster for rendering. 
    // The width and height will both be halfed, so the area resolution is quartered.
    // i.e. 640x360 -> 320x180
    const bool HALF_SIZE = true;
    const bool FULL_SIZE = false;
    std::string generateTML(const std::string& configFile, bool halfSize=false);
};
