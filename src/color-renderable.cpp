#include "color-renderable.hpp"
#include "gpu-frame-impl.hpp"
#include "gpu-frame-cache.hpp"
#include <stdexcept>
#include <boost/algorithm/string.hpp>

using namespace CCPlus;
using namespace cv;

ColorRenderable::ColorRenderable(const std::string& uri) {
    this->uri = uri;
    std::string s = "";
    try {
        s = this->uri.substr(8);
    } catch (std::out_of_range _ignore) {
        log(logERROR) << "Invalid uri for color renderable " << uri;
        return;
    }
    std::vector<std::string> strs;
    boost::split(strs, s, boost::is_any_of(","));
    if (strs.size() != 5) {
        log(logERROR) << "Invalid uri for color renderable_ " << uri;
        return;
    }
    this->width = std::atoi(strs[0].c_str());
    this->height = std::atoi(strs[1].c_str());
    this->r = std::atof(strs[2].c_str());
    this->g = std::atof(strs[3].c_str());
    this->b = std::atof(strs[4].c_str());
}

float ColorRenderable::getDuration() {
    return 1000000;
}

void ColorRenderable::prepare() {
}

GPUFrame ColorRenderable::getGPUFrame(float) {
    if (cache) return cache;
    int pw = nearestPOT(width);
    int ph = nearestPOT(height);
    cache = GPUFrameCache::alloc(pw, ph);
    cache->ext.scaleAdjustX = width  * 1.0f / pw;
    cache->ext.scaleAdjustY = height * 1.0f / ph;
    cache->bindFBO(false);
    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
    return cache;
}

void ColorRenderable::release() {
    cache = GPUFrame();
}

int ColorRenderable::getWidth() const {
    return width;
}

int ColorRenderable::getHeight() const {
    return height;
}
