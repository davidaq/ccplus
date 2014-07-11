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
#include <utility>
#include <cstdio>
#include <iostream>

#ifdef IN_CCPLUS_PRIVATE_CONTEXT
//#include <boost>
#endif

// public headers
#include "object.hpp"
#include "context.hpp"
#include "renderable.hpp"
#include "tmlreader.hpp"
#include "composition.hpp"
#include "layer.hpp"
