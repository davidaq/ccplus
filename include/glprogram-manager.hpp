#pragma once
#include "global.hpp"

namespace CCPlus {
    enum AttribLocation {
        ATTRIB_VERTEX_POSITION = 0,
    };
}

class CCPlus::GLProgramManager : public Object {
public:
    static CCPlus::GLProgramManager* getManager();
    void clean();
    GLuint getProgram(
            const std::string& name,
            const std::string& vshader,
            const std::string& fshader);

private: 
    std::map<std::string, GLuint> programPool;
};
