CCPlus Overview
===============

CCPlus is an engine for defining and rendering video compositions. 
The purpose of this project is to implement a video rendering 
engine acting as a lightweight alternative for Adobe After Effects
render engine. The objective is to implement a video rendering 
engine that will not only work on PCs or servers but also on mobile
devices *(specifially android and iOS systems)*.

This engine will be used on mobile device to generate preview 
quality videos and final quality on servers. Another project will
be found to provide distributed video render service using this 
engine.

Technology Selections
---------------------

 - **C++(version C++11)** - main programming language
 - **Boost** - the one and only depended library for the core part
 - **GYP** - project organization and multi-platform build
 - **JSON** - serialize persisted data
 - **MinGW** - support for windows
 - **FFmpeg** - media decode/encode
 - **OpenCV** - image manipulation

Platform support
----------------

CCPlus provide (or will provide) each of the following platforms
with a working component (executables or libraries) providing an 
interface to access the features of CCPlus.

 - **Windows/Linux/OS X** - a command line executable tool
 - **Android** - a *.so dynamic library 
 - **iOS** - a framework package

Differences with Adobe After Effects Render Engine
--------------------------------------------------

 - no gui, every operation can only be done by calling api/cli
 - less supported effects, only the basic ones will be supported
 - no inline expression support
 - available on iOS/Android/Windows/Linux/OS X platforms
 - human readable file structures, allowing easy modification
 - less startup overhead, making it easier for distributed computing
