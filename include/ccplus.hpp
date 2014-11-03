/***********************************************************************************
 * CCPlus Render Engine API header
 * This is the one and only header file exposed to higher level application.
 *
 * Every function listed in this header file are APIs to leverage rendering
 * ability of this engine.
 ***********************************************************************************/

#pragma once
namespace CCPlus {
    // @ easy one shot render shortcut
    void go(const std::string& tmlPath, const std::string& outputPath, int fps);

    // @ initialize the rendering context
    void initContext(const std::string& tmlPath, const std::string& outputPath, int fps);

    // @ release the rendering context
    void releaseContext();

    // @ start the render
    void render();

    // @ start the encode
    void encode();

    // @ get the total number of frames for the current render context
    int numberOfFrames();

    // @ generate a timeline from a configuration file
    void fillTML(const std::string& configFile, const std::string& output);
    // @ Generate a timeline dynamically
    void generateTML(const std::string& configFile, const std::string& output);

    // @ Wait for render
    void waitRender();
};
