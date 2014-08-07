#pragma once

#include "config.hpp"

// ccplus classes predifinitions
namespace CCPlus {

    class Context;
    
    class Renderable;

    class TMLReader;

    class Composition;

    class Layer;
    
    class CompositionDependency;
    
    class Object;

    class Frame;
}

// system dependencies
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <complex>
#include <functional>

struct DoubleLess {
    bool operator()(float left, float right) const
    {
        //return left < right;
        return (std::abs(left - right) > 0.00001) && (left < right);
    }
}; 
typedef std::map<float, std::vector<float>, DoubleLess> Property;
typedef std::map<std::string, Property> PropertyMap;

#ifdef IN_CCPLUS_PRIVATE_CONTEXT

#endif

// public headers
#include "object.hpp"
#include "context.hpp"
#include "renderable.hpp"
#include "tmlreader.hpp"
#include "composition.hpp"
#include "layer.hpp"
#include "frame.hpp"
#include "filter.hpp"
#include "time.h"
#include "logger.hpp"
//#include "file-manager.hpp"

#define PASS printf("passed line %d in %s at %lums\n", __LINE__, __FILE__, clock() * 1000 / CLOCKS_PER_SEC);
