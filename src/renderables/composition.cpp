#include "composition.hpp"
#include "parallel-executor.hpp"
#include "global.hpp"
#include "utils.hpp"
#include "image-renderable.hpp"

#include <list>
#include <pthread.h>
#include <algorithm>

using namespace CCPlus;

Composition::Composition(
        CCPlus::Context* ctx, std::string _name,
        float _dur, int _width, int _height) :
    AnimatedRenderable(ctx), 
    name(_name), width(_width), height(_height), duration(_dur)
{
    pthread_mutex_init(&renderedLock, 0);
}

Composition::~Composition() {
}

const std::string& Composition::getName() const {
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

void Composition::setForceRenderToFile(bool renderToFile) {
    this->renderToFile = renderToFile;
}

void Composition::renderPart(float start, float duration) {
    float inter = 1.0 / context->getFPS();

    ParallelExecutor executor(CCPlus::CONCURRENT_THREAD);

    float time_slice = std::max(2.0, 
            duration / CCPlus::CONCURRENT_THREAD * 1.0);

    float end = start + duration + inter;
    for (float t = start; t < end; t += time_slice) {
        float _start = t;
        float _end = std::min(t + time_slice, end);

        // Cut time to slices
        auto render = [this, inter, _start, _end] () {
            float last_t = -1;
            for (float t = _start; t <= _end + inter; t += inter) {
                int f = this->getFrameNumber(t);

                // Check whether rendered
                pthread_mutex_lock(&renderedLock);
                if(rendered.count(f)) {
                    pthread_mutex_unlock(&renderedLock);
                    continue;
                }
                pthread_mutex_unlock(&renderedLock);

                std::string fp = getFramePath(f);

                Frame ret;

                if (last_t != -1 && 
                        layers.size() > 1 && 
                        this->still(last_t, t)) {

                    //L() << last_t << " and " << t;
                    ret = this->getFrame(last_t);
                } else {
                    bool first = true; 
                    for (Layer& l : layers) {
                        Frame frame = l.applyFiltersToFrame(t);
                        if (frame.empty()) continue;
                        if (first) {
                            ret = frame;
                            first = false;
                        } else {
                            profileBegin(MergeFrame);
                            ret.mergeFrame(frame);
                            profileEnd(MergeFrame);
                        }
                    }
                }
                if(renderToFile)
                    ret.write(fp, 80, false);
                else 
                    ret.write(fp);

                // Save ret to storagePath / name_tmp
                pthread_mutex_lock(&renderedLock);
                rendered.insert(f);
                pthread_mutex_unlock(&renderedLock);

                last_t = t;
            }
        };
        executor.execute(render);
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

int Composition::getTotalNumberOfFrame() const {
    return duration * this->context->getFPS() - 1;
}

const std::string Composition::getPrefix() const {
    return generatePath(this->context->getStoragePath(), this->uuid + "_");
}

bool Composition::still(float t1, float t2) {
    for (auto& l : layers) {
        if (l.visible(t1) != l.visible(t2))
            return false;
        if (!l.visible(t1)) continue;
        if (!l.getRenderObject()->still(t1, t2))
            return false;
        const PropertyMap& mp = l.getProperties();
        for (auto& kv : mp) {
            std::string name = kv.first;
            if (name == "volume") continue;
            if (l.interpolate(name, t1) != l.interpolate(name, t2))
                return false;
        }
    }
    return true;
}
