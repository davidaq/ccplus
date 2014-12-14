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
    void setAssetsPath(const std::string&);     // defaults to ""
    void setCollectorThreadsNumber(int);        // defaults to 2
    void setFrameRate(int);                     // defaults to 18
    int getFrameRate();
    void setOutputPath(const std::string&);     // defaults to "tmp"
    
    const int PREVIEW_MODE = 0;
    const int FINAL_MODE   = 1;
    void setRenderMode(int);                    // defaults to FINAL_MODE

    // @ easy one shot render shortcut
    // pass thorugh init, render, wait, release procedure 
    void go(const std::string& tmlPath);

    // @ initialize the rendering context
    void initContext(const std::string& tmlPath);

    // @ release the rendering context, terminates the rendering process if running
    // will not release preserved renderables in preview mode by default
    // but the cache can be cleared by passing true to forceClearCache
    void releaseContext(bool forceClearCache=false);

    // @ start the asynchronized render process
    // when in preview mode, a list of zim files will be generated to `outputPath` directory
    // when in final mode, a single video file will be generated to `outputPath`
    // mind that we only support encoding mp4 files
    void render();

    // @ Wait for render thread to finish or terminate
    void waitRender();

    // @ get the render progress in percentage
    // 100 means done or there is no render activity
    // 99 means in encoding procces
    int getRenderProgress();

    // @ get the total number of frames for the current render context
    int numberOfFrames();
    
    // @ Generate a timeline file using a dynamical template
    // may use the half size hint to generate a smaller resolution
    // version of the timeline, which is faster for rendering. 
    // The width and height will both be halfed, so the area resolution
    // is quartered. i.e. 640x360 -> 320x180
    const bool HALF_SIZE = true;
    const bool FULL_SIZE = false;
    std::string generateTML(const std::string& configFile, bool halfSize=false);

    // @ Gernerates images to be used as covers if there is a #cover scene in the tml
    // Results will be save to @outputPath
    void generateCoverImages(const std::string& tmlPath, const std::string& outputPath);
    

    /******************************************************
     * Deprecated - The following functions are deprecated
     * they are legacy features or for testing purpose
     *****************************************************/

    // @ generate a timeline from a configuration file using statical templates
    void fillTML(const std::string& configFile, const std::string& output);
};
