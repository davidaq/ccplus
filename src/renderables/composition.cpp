#include "composition.hpp"
#include "parallel-executor.hpp"
#include "global.hpp"
#include "utils.hpp"
#include "image-renderable.hpp"
#include "file-manager.hpp"

#include <list>
#include <pthread.h>
#include <algorithm>
#include <utility>

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
    std::vector<std::pair<float, float>> fromto;
    if (to == from) return dps;
    if (to - from >= this->duration) {
        fromto.push_back(std::make_pair(0, this->duration));
    } else if (to - from < this->duration && to > from) {
        fromto.push_back(std::make_pair(from, to));
    } else {
        fromto.push_back(std::make_pair(0, to));
        fromto.push_back(std::make_pair(from, this->duration));
    }
    for(Layer layer : getLayers()) {
        for (auto& ft : fromto) {
            float from = ft.first;
            float to = ft.second;
            if(layer.getTime() < to && 
               layer.getTime() + layer.getDuration() > from) {
                // Respect to the composition timeline
                float layerFrom = std::max(from, layer.getTime());
                float layerTo = std::min(to, 
                        layer.getTime() + layer.getDuration());
                CompositionDependency dp;
                dp.renderable = layer.getRenderObject();
                //dp.from = layer.getStart(); 
                //dp.to = layer.getStart() + layer.getLast();
                // Partial dependency
                dp.from = layer.getStart() + 
                    (layerFrom - layer.getTime()) / 
                    layer.getDuration() * layer.getLast();
                dp.to = layer.getStart() + 
                    (layerTo - layer.getTime()) / 
                    layer.getDuration() * layer.getLast();
                dps.push_back(dp);
            }
        }
    }
    return dps;
}

bool compositionDependencyCompare(CompositionDependency c1, CompositionDependency c2) {
    return c1.from < c2.from || (c1.from == c2.from && c1.to < c2.to);
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
        //candidate.from = 0;
        candidate.from = map[r][0].from;
        candidate.to = -1000000000;
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
    int step = std::max((int)1.0 * context->getFPS(), 
            totalFrame / CCPlus::CONCURRENT_THREAD);
    step = std::min((int) 2.0 * context->getFPS(), step);
    int i = startFrame;
    while (i <= endFrame) {
        float _startFrame = i;
        float _endFrame = std::min(i + step, endFrame);
        i = _endFrame + 1;

        // Cut frames to pieces
        auto render = [this, inter, _startFrame, _endFrame] () {
            int lastFrame = _startFrame - 1;
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
                if (rendered.count(lastFrame) && this->still(last_t, now_t)) {
                    if(renderToFile) {
                        ret = this->getFrameByNumber(lastFrame);
                    } else {
                        FileManager::getInstance()->addLink(
                                this->getFramePath(f), 
                                this->getFramePath(lastFrame));
                        flag = true;
                    }
                } else {
                    bool first = true; 
                    bool buffer[layers.size()];
                    Frame* bufferFrame = new Frame[layers.size()];
                    memset(buffer, false, layers.size());
                    for (int i = layers.size() - 1; i >= 0; i--) {
                        Layer l = layers[i];
                        Frame frame = buffer[i] ? 
                            bufferFrame[i] :
                            l.applyFiltersToFrame(now_t);
                        if (frame.empty() || !l.show()) continue;
                        /*
                         * handle trkmat
                         */
                        if (l.getTrackMatte() && i != 0) {
                            buffer[i - 1] = true;
                            bufferFrame[i - 1] = layers[i - 1].applyFiltersToFrame(now_t);
                            frame.trackMatte(bufferFrame[i - 1], l.getTrackMatte());
                        }
                        /*
                         * Normal merge
                         */
                        if (first) {
                            ret = frame;
                            // TODO: the order here might be problematic
                            if (l.getBlendMode() != 0) {
                                ret.mergeFrame(
                                        Frame::emptyFrame(
                                            ret.getWidth(), 
                                            ret.getHeight()), 
                                        l.getBlendMode());
                            }
                            first = false;
                        } else {
                            frame.mergeFrame(ret, l.getBlendMode());
                            ret = frame;
                        }
                    }
                    delete [] bufferFrame;
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

