#pragma once

namespace CCPlus {
    class Object;
    class Lock;
    class Semaphore;
    class Context;
    class Layer;
    class Profiler;
    class Logger;
    class VideoDecoder;
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
    typedef boost::shared_ptr<GPUFrameImpl> GPUFrame;
}
#include "platform.hpp"
