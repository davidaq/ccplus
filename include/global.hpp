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
    class GPUFrame;
    class FootageCollector;
    class Filter;

    class Renderable;
    class Composition;
    class ImageRenderable;
    class VideoRenderable;
};

#include <cstdio>
#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "externals/gl2.h"
#include "externals/gl2ext.h"

#include "config.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "logger.hpp"
