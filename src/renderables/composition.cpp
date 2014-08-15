#include "composition.hpp"
#include "parallel-executor.hpp"
#include "global.hpp"
#include "utils.hpp"
#include "image-renderable.hpp"
#include "file-manager.hpp"

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
    //log(logDEBUG) << "\t" << start << " " << duration;
    float inter = 1.0 / context->getFPS();

    ParallelExecutor executor(CCPlus::CONCURRENT_THREAD);

    int startFrame  = this->getFrameNumber(start);
    int endFrame = this->getFrameNumber(start + duration);
    int totalFrame = endFrame - startFrame + 1;
    int step = std::max((int)2.0 * context->getFPS(), 
            totalFrame / CCPlus::CONCURRENT_THREAD);
    step = std::min((int) 5.0 * context->getFPS(), step);
    int i = startFrame;
    while (i <= endFrame) {
        float _startFrame = i;
        float _endFrame = std::min(i + step, endFrame);
        i = _endFrame + 1;

        // Cut frames to pieces
        auto render = [this, inter, _startFrame, _endFrame] () {
            int lastFrame = -1;
            for (int f = _startFrame; f <= _endFrame; f++) {

                // Check whether rendered
                pthread_mutex_lock(&renderedLock);
                if(rendered.count(f)) {
                    pthread_mutex_unlock(&renderedLock);
                    continue;
                }
                pthread_mutex_unlock(&renderedLock);

                std::string fp = getFramePath(f);

                Frame ret;

                float last_t = this->getFrameTime(lastFrame);
                float now_t = this->getFrameTime(f);

                bool flag = false;
                if (lastFrame != -1 && this->still(last_t, now_t)) {
                    //L() << this->getName() << last_t << " and " << now_t;
                    FileManager::getInstance()->addLink(
                            this->getFramePath(f), 
                            this->getFramePath(lastFrame));
                    flag = true;
                } else {
                    bool first = true; 
                    for (Layer& l : layers) {
                        Frame frame = l.applyFiltersToFrame(now_t);
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

                if (!flag) {
                    if(renderToFile)
                        ret.write(fp, 80, false);
                    else 
                        ret.write(fp);
                }

                // Save ret to storagePath / name_tmp
                pthread_mutex_lock(&renderedLock);
                rendered.insert(f);
                pthread_mutex_unlock(&renderedLock);

                lastFrame = f;
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
            //if (name == "volume") continue;
            if (l.interpolate(name, t1) != l.interpolate(name, t2))
                return false;
        }
    }
    return true;
}
