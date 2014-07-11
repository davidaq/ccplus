#include <composition.hpp>
#include <list>

using namespace CCPlus;

Composition::Composition(
        CCPlus::Context* ctx, std::string _name,
        float _dur, float _width, float _height) :
    Renderable(_dur, _width, _height), context(ctx), name(_name) 
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
    // Merge dependency
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


