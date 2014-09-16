#pragma once

#include "context.hpp"

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

struct CCPlus::ExtraContext {
    FT_Library freetype;
    FT_Face font;
};
