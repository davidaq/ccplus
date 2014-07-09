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
 - **boost** - the one and only depended library for the core part
 - **GYP** - project organization and multi-platform build
 - **JSON** - serialize persisted data

Platform support
----------------

CC Plus provide (or will provide) each of the following platforms
with a working component (executables or libraries) used to render
videos

 - ***nix PC/MAC/Server** - a command line executable tool
 - **android** - a *.so dynamic library 
 - **iOS** - a framework package
