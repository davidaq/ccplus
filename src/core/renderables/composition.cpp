#include "composition.hpp"
#include "global.hpp"
#include <list>

using namespace CCPlus;

Composition::Composition(
        CCPlus::Context* ctx, std::string _name,
        float _dur, int _width, int _height) :
    AnimatedRenderable(ctx), 
    name(_name), width(_width), height(_height), duration(_dur)
{
}

std::string Composition::getName() const {
    return name;
}

std::vector<CompositionDependency> Composition::directDependency(float from, float to) const {
    std::vector<CompositionDependency> dps;
    for(Layer layer : getLayers()) {
        if(layer.getTime() < to && layer.getTime() + layer.getDuration() > from) {
            CompositionDependency dp;
            dp.renderable = layer.getRenderObject();
            dp.from = layer.getStart();
            dp.to = layer.getStart() + layer.getLast();
            dps.push_back(dp);
        }
    }
    return dps;
}

bool compositionDependencyCompare(CompositionDependency c1, CompositionDependency c2) {
    return c1.from > c2.from;
}

std::vector<CompositionDependency> Composition::fullOrderedDependency(float from, float to) const {
    // do a tree traverse
    std::list<CompositionDependency> ordered;
    std::list<CompositionDependency> unexamined;
    CompositionDependency dp;
    dp.renderable = (Renderable*) this;
    dp.from = from;
    dp.to = to;
    unexamined.push_back(dp);
    while(!unexamined.empty()) {
        CompositionDependency dep = unexamined.front();
        unexamined.pop_front();
        ordered.push_front(dep);
        Composition* comp = dynamic_cast<Composition*>(dep.renderable);
        if(comp != 0) {
            for(CompositionDependency dep : comp->directDependency(dep.from, dep.to)) {
                unexamined.push_back(dep);
            }
        }
    }
    // Merge dependency of the same renderables
    std::list<Renderable*> renderableDependOrder;
    std::map<Renderable*,std::vector<CompositionDependency> > map;
    for(CompositionDependency dep : ordered) {
        if(!map.count(dep.renderable)) {
            renderableDependOrder.push_front(dep.renderable);
        }
        map[dep.renderable].push_back(dep);
    }
    ordered.clear();
    for(Renderable* r : renderableDependOrder) {
        std::sort(map[r].begin(), map[r].end(), compositionDependencyCompare);
        CompositionDependency candidate;
        candidate.renderable = r;
        candidate.from = 0;
        candidate.to = 0;
        for(CompositionDependency dep : map[r]) {
            if(dep.from > candidate.to) {
                if(candidate.to > candidate.from) {
                    ordered.push_back(candidate);
                }
                candidate.from = dep.from;
                candidate.to = dep.to;
            } else {
                candidate.to = dep.to;
            }
        }
    }
    return std::vector<CompositionDependency>(ordered.begin(), ordered.end());
}

std::vector<Layer> Composition::getLayers() const {
    return layers;
}

void Composition::putLayer(const Layer& layer) {
    layers.push_back(layer);
}

void Composition::renderPart(float start, float duration) {
    float inter = 1.0 / context->getFPS();

    for (float t = 0; t <= start + duration; t += inter) {
        int f = getFrameNumber(t);
        if(rendered.count(f))
            continue;
        std::string fp = getFramePath(f);

        Image ret = Image::emptyImage(width, height);
        for (Layer& l : layers) {
            Image img = l.applyFiltersToFrame(t);
            ret.overlayImage(img);
        }
        // Save ret to storagePath / name_tmp
        ret.write(fp);
        rendered.insert(f);
    }
}

int Composition::getWidth() const {
    return width;
}

int Composition::getHeight() const {
    return height;
}

float Composition::getDuration() const {
    return duration;
}
