#pragma once

// ccplus classes predifinitions
namespace CCPlus {
    
    class Context;
    
    class Renderable;

    class TMLReader;

    class Composition;

    class Layer;
    
    class CompLayer;

    class TextLayer;
    
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

typedef std::map<float, std::vector<float>> Property;
#ifdef IN_CCPLUS_PRIVATE_CONTEXT
//typedef std::map<float, std::vector<float>> Property;
#endif

// public headers
#include "object.hpp"
#include "context.hpp"
#include "renderable.hpp"
#include "tmlreader.hpp"
#include "composition.hpp"
#include "layer.hpp"

