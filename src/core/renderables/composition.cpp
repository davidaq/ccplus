#include "composition.hpp"
#include "parallel-executor.hpp"
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

Composition::~Composition() {
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
    return c1.from < c2.from;
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
            renderableDependOrder.push_back(dep.renderable);
        }
        map[dep.renderable].push_back(dep);
    }
    ordered.clear();
    for(Renderable* r : renderableDependOrder) {
        std::sort(map[r].begin(), map[r].end(), compositionDependencyCompare);
        CompositionDependency candidate;
        candidate.renderable = r;
        candidate.from = 0;
        candidate.to = -1;
        for(CompositionDependency dep : map[r]) {
            if(dep.from > candidate.to) {
                if(candidate.to > candidate.from) {
                    ordered.push_back(candidate);
                }
                candidate.to = dep.to;
                candidate.from = dep.from;
            } else {
                candidate.to = dep.to;
            }
        }
        if(candidate.to > candidate.from) {
            ordered.push_back(candidate);
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

    ParallelExecutor executor(CCPlus::CONCURRENT_THREAD);
    // Plus an inter to make sure no lost frame
    for (float t = start; t <= start + duration + inter; t += inter) {
        //printf("t = %f\n", t);
        int f = getFrameNumber(t);
        if(rendered.count(f))
            continue;
        std::string fp = getFramePath(f);

        auto render = [fp,t,this] {
            Frame ret = Frame::emptyFrame(width, height);
            for (Layer& l : layers) {
                Frame frame = l.applyFiltersToFrame(t);
                // In some cases it will be empty
                ret.mergeFrame(frame);
            }
            ret.write(fp);
        };
        executor.execute(render);
        // Save ret to storagePath / name_tmp
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
