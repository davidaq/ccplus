#pragma once

namespace CCPlus {
    class Object;
    class Lock;
    class ScopeHelper;
    class Semaphore;
    class Context;
    class Layer;
    class Profiler;
    class Logger;
    class IVideoDecoder;
    class VideoDecoder;
    class IVideoEncoder;
    class VideoEncoder;
    class ParallelExecutor;
    class TMLReader;
    class Frame;
    class GPUFrameImpl;
    class GPUFrameCache;
    class GPUDoubleBuffer;
    class FootageCollector;
    class DependencyWalker;
    class Filter;
    class GLProgramManager;

    class Renderable;
    class Composition;
    class ImageRenderable;
    class VideoRenderable;
    class ColorRenderable;
};

#include <cstdio>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <ft2build.h>
#include <boost/shared_ptr.hpp>
#include FT_FREETYPE_H
#include "externals/gl2.h"
#include "externals/gl2ext.h"

#include "config.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "profile.hpp"

namespace CCPlus {
    extern bool appPaused;
    extern uint16_t gpuContextCounter;
#include "gpu-frame-ref.hpp"
    typedef boost::shared_ptr<IVideoDecoder> IVideoDecoderRef;
};

#include "platform.hpp"
