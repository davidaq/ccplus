#pragma once

// ccplus classes predifinitions
namespace CCPlus {
    
    class Context;
    
    class Renderable;

    class TMLReader;

    class Composition;

    class Layer;
    
    class CompositionDependency;
    
    class Object;

    // TODO: other classes
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
#include "image.hpp"
#include "filter.hpp"
