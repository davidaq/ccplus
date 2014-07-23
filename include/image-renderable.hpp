#pragma once

#include "renderable.hpp"

namespace CCPlus {
    class ImageRenderable;
}

// @ static image, period.
class CCPlus::ImageRenderable : public CCPlus::Renderable {
public:
    ImageRenderable(CCPlus::Context* context, const std::string& uri);
};

