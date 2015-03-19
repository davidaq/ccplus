#-------------------------------------------------
#
# Project created by QtCreator 2015-03-13T17:20:36
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = ccplus-qt
CONFIG   += console c++11 thread opengl
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    platform.cpp \
    ../../../src/ccplay.cpp    \
    ../../../src/ccplus-base.cpp   \
    ../../../src/ccplus-filltml.cpp    \
    ../../../src/ccplus.cpp    \
    ../../../src/color-renderable.cpp  \
    ../../../src/composition.cpp   \
    ../../../src/config.cpp    \
    ../../../src/context.cpp   \
    ../../../src/dependency-walker.cpp \
    ../../../src/externals/giflib/dgif_lib.c \
    ../../../src/externals/giflib/gif_err.c \
    ../../../src/externals/giflib/gifalloc.c \
    ../../../src/externals/lua/lapi.c \
    ../../../src/externals/lua/lauxlib.c \
    ../../../src/externals/lua/lbaselib.c \
    ../../../src/externals/lua/lbitlib.c \
    ../../../src/externals/lua/lcode.c \
    ../../../src/externals/lua/lcorolib.c \
    ../../../src/externals/lua/lctype.c \
    ../../../src/externals/lua/ldblib.c \
    ../../../src/externals/lua/ldebug.c \
    ../../../src/externals/lua/ldo.c \
    ../../../src/externals/lua/ldump.c \
    ../../../src/externals/lua/lfunc.c \
    ../../../src/externals/lua/lgc.c \
    ../../../src/externals/lua/linit.c \
    ../../../src/externals/lua/liolib.c \
    ../../../src/externals/lua/llex.c \
    ../../../src/externals/lua/lmathlib.c \
    ../../../src/externals/lua/lmem.c \
    ../../../src/externals/lua/loadlib.c \
    ../../../src/externals/lua/lobject.c \
    ../../../src/externals/lua/lopcodes.c \
    ../../../src/externals/lua/loslib.c \
    ../../../src/externals/lua/lparser.c \
    ../../../src/externals/lua/lstate.c \
    ../../../src/externals/lua/lstring.c \
    ../../../src/externals/lua/lstrlib.c \
    ../../../src/externals/lua/ltable.c \
    ../../../src/externals/lua/ltablib.c \
    ../../../src/externals/lua/ltm.c \
    ../../../src/externals/lua/lua.c \
    ../../../src/externals/lua/luac.c \
    ../../../src/externals/lua/lundump.c \
    ../../../src/externals/lua/lvm.c \
    ../../../src/externals/lua/lzio.c \
    ../../../src/externals/lz4.c \
    ../../../src/externals/lz4_hc.c \
    ../../../src/externals/triangulate.cpp \
    ../../../src/filter.cpp \
    ../../../src/filters/4color.cpp \
    ../../../src/filters/4corner.cpp \
    ../../../src/filters/fill.cpp \
    ../../../src/filters/gaussian.cpp \
    ../../../src/filters/grayscale.cpp \
    ../../../src/filters/hsl.cpp \
    ../../../src/filters/mask.cpp \
    ../../../src/filters/opacity.cpp \
    ../../../src/filters/ramp.cpp \
    ../../../src/filters/transform.cpp \
    ../../../src/filters/volume.cpp \
    ../../../src/footage-collector.cpp \
    ../../../src/frame.cpp \
    ../../../src/gif-renderable.cpp \
    ../../../src/glprogram-manager.cpp \
    ../../../src/gpu-frame-cache.cpp \
    ../../../src/gpu-frame-impl.cpp \
    ../../../src/gpu-frame-ref.cpp \
    ../../../src/image-renderable.cpp \
    ../../../src/layer.cpp \
    ../../../src/logger.cpp \
    ../../../src/object.cpp \
    ../../../src/parallel-executor.cpp \
    ../../../src/profile.cpp \
    ../../../src/render.cpp \
    ../../../src/renderable.cpp \
    ../../../src/shadertest.cpp \
    ../../../src/text-renderable.cpp \
    ../../../src/tmlreader.cpp \
    ../../../src/utils.cpp \
    ../../../src/video-decoder.cpp \
    ../../../src/video-encoder.cpp \
    ../../../src/video-renderable.cpp

HEADERS += \
    qtport.h

INCLUDEPATH += \
    $$PWD/../../../include \
    $$PWD/../../../include/externals/lua \
    $$PWD/../../../dependency/boost \
    $$PWD/../../../dependency/ffmpeg/headers \
    $$PWD/../../../dependency/freetype \
    $$PWD/../../../dependency/opencv/headers

include(platform.pro)
