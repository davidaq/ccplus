#include <fstream>
#include "tmlreader.hpp"
#include "context.hpp"
#include "composition.hpp"
#include "image-renderable.hpp"
#include "video-renderable.hpp"
#include "text-renderable.hpp"
#include "gif-renderable.hpp"
#include "color-renderable.hpp"
#include "ccplus.hpp"
#include "queue"

using namespace CCPlus;

Composition* TMLReader::read(const std::string& s) const {
    std::ifstream fstream(s, std::ios::in);
    if (!fstream.is_open()) {
        log(logFATAL) << "Couldn't open file: " << s;
    }
    using boost::property_tree::ptree;
    ptree pt;
    read_json(s, pt);
    fstream.close();

    std::string main_name = pt.get<std::string>("main");
    int fps = pt.get<int>("fps", frameRate);
    if(renderMode == PREVIEW_MODE) {
        fps = pt.get<int>("preview_fps", fps);
    }
    setFrameRate(fps);
    Context* ctx = Context::getContext();
    try {
        for (auto& child : pt.get_child("bgm_volume")) {
            float t = std::atof(child.first.data());
            float v = std::atof(child.second.data().c_str());
            ctx->bgmVolumes.push_back(std::make_pair(t, v));
        }
    } catch(...) {
        ctx->bgmVolumes.push_back(std::make_pair(0, 1));
    }
    auto pairCompare = [] (const std::pair<float, float>& a, const std::pair<float, float>& b) {
        return a.first < b.first;
    };
    std::sort(ctx->bgmVolumes.begin(), ctx->bgmVolumes.end(), pairCompare);
    
    std::queue<std::string> queue;
    queue.push(main_name);
    while (!queue.empty()) {
        std::string name = queue.front();
        bool ok = initComposition(name, pt.get_child("compositions." + name));
        queue.pop();
        if(!ok)
            continue;
        for (auto& child : pt.get_child("compositions." + name + ".layers")) {
            std::string uri = child.second.get("uri", "");
            if (stringStartsWith(uri, "composition://")) {
                std::string newname = uri.substr(14);
                queue.push(newname);
            }
        }
    }
    
    return (Composition*)Context::getContext()->getRenderable("composition://" + main_name);
}

bool TMLReader::initComposition(const std::string& name, const boost::property_tree::ptree& pt) const {
    if (Context::getContext()->hasRenderable("composition://" + name)) {
        return false;
    }
    Composition* comp = new Composition(
            pt.get("duration", 0.0f),
            pt.get("resolution.width", 0.0),
            pt.get("resolution.height", 0.0));

    for (auto& child: pt.get_child("layers")) {
        auto& t = child.second;
        Layer compLayer = initLayer(t, comp->width, comp->height);
        comp->appendLayer(compLayer);
    }

    Context::getContext()->putRenderable("composition://" + name, comp);
    return true;
}

std::map<std::string, Property> TMLReader::readProperties(const boost::property_tree::ptree& pt) const {
    std::map<std::string, Property> ret;
    for (auto& child: pt.get_child("properties")) {
        auto name = child.first.data();
        for (auto& pc : pt.get_child(std::string("properties.") + name)) {
            float t = std::atof(pc.first.data());
            for (auto& v : pc.second) {
                float val = std::atof(v.second.data().c_str());
                ret[name][t].push_back(val);
            }
        }
    }
    return ret;
}

Layer TMLReader::initLayer(const boost::property_tree::ptree& pt, int width, int height) const {
    std::string uri = pt.get("uri", "");
    if (!Context::getContext()->hasRenderable(uri)) {
        Renderable* renderable = 0;
        if (stringStartsWith(uri, "xfile://") || stringStartsWith(uri, "file://")) {
            std::map<std::string, std::function<Renderable*(const std::string&)> > extMap;
            auto imageExt = [](const std::string& uri) {
                return new ImageRenderable(uri);
            };
            extMap["jpg"]       = imageExt;
            extMap["jpeg"]      = imageExt;
            extMap["png"]       = imageExt;
            extMap["bmp"]       = imageExt;
            auto gifExt = [](const std::string& uri) {
                return new GifRenderable(uri);
            };
            extMap["gif"]       = gifExt;
            auto audioExt = [](const std::string& uri) {
                return new VideoRenderable(uri, true);
            };
            extMap["mp3"]       = audioExt;
            extMap["aac"]       = audioExt;
            extMap["flac"]      = audioExt;
            extMap["wav"]       = audioExt;
            extMap["asf"]       = audioExt;
            extMap["wma"]       = audioExt;
            extMap["ogg"]       = audioExt;
            extMap["rm"]        = audioExt;
            extMap["caf"]       = audioExt;
            auto videoExt = [](const std::string& uri) {
                return new VideoRenderable(uri, false);
            };
            extMap["mp4"]       = videoExt;
            extMap["mov"]       = videoExt;
            extMap["rmvb"]      = videoExt;
            extMap["flv"]       = videoExt;
            extMap["f4v"]       = videoExt;
            extMap["webm"]      = videoExt;
            extMap["ogv"]       = videoExt;
            // Unguessable from extension, probe file
            auto defaultExt = [](const std::string& uri) {
                Renderable* ret = 0;
                if(cv::imread(Renderable::parseUri2File(uri)).empty()) {
                    ret = new VideoRenderable(uri, false);
                } else {
                    ret = new ImageRenderable(uri);
                }
                return ret;
            };
            extMap["default"] = defaultExt;

            std::string ext;
#ifdef __IOS__
            const char* alext = assetsLibraryExt(uri.c_str());
            if(alext)
                ext = alext;
#endif
            if(ext.empty()) {
                size_t dotPos = uri.find_last_of('.');
                ext = dotPos != std::string::npos ? uri.substr(dotPos + 1) : "";
                ext = toLower(ext);
            }
            
            log(logINFO) << "Got file extention: " << ext;
            if(!extMap.count(ext)) {
                ext = "default";
                std::string path = uri;
                if (stringStartsWith(path, "file://")) 
                    path = uri.substr(7);
                path = Context::getContext()->getFootagePath(path);
                FILE* fp = fopen(path.c_str(), "rb");
                if(fp) {
                    char buff[4];
                    fread(buff, 1, 3, fp);
                    fclose(fp);
                    buff[3] = 0;
                    if(strcmp(buff, "GIF") == 0) {
                        ext = "gif";
                    }
                }
            }
            if (!Context::getContext()->hasRenderable(uri)) {
                renderable = extMap[ext](uri);
            } else {
                renderable = Context::getContext()->getRenderable(uri);
            }
        } else if (stringStartsWith(uri, "text://")) {
            renderable = new TextRenderable(pt);
        } else if (stringStartsWith(uri, "color://")) {
            renderable = new ColorRenderable(uri);
        } else if (!stringStartsWith(uri, "composition://")) {
            log(logWARN) << "Ignore unkwown footage type " << uri;
        }
        if(renderable) {
            Context::getContext()->retain(renderable);
            Context::getContext()->putRenderable(uri, renderable);
        }
    }
    int blendMode = pt.get("blend", 0);
    int trkMat = pt.get("trkMat", 0);
    bool showup = pt.get("visible", true);
    Layer l = Layer(
            uri, pt.get("time", 0.0f), pt.get("duration", 0.0f),
            pt.get("start", 0.0f), pt.get("last", 0.0f), width, height,
            blendMode, trkMat, showup, pt.get("motionBlur", false), 
            pt.get("rawTransform", false));
    l.setProperties(readProperties(pt));
    return l;
}

