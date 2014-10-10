#pragma once

#include <vector>

namespace CCPlus {
    class FootageCollector;
    class Composition;
    class Object;
    class Layer;
}

class CCPlus::FootageCollector : public CCPlus::Object {

public:
    FootageCollector(const Composition* comp);
    ~FootageCollector();

    void prepare();

    const std::vector<FootageDependency>& getDependency() const; 

    struct FootageDependency {
        Renderable* renderable;
        // @time is to global time
        float time;
        // @ Start and @end are to the local time of 
        // the @renderable
        float start, end;
        bool operator<(const FootageDependency& f) const {
            return time < f.time;
        }
    };

protected:
    void mergeDependency(Layer* l, const std::vector<FootageDependency>&);
    std::vector<FootageDependency> dependencies;
}
