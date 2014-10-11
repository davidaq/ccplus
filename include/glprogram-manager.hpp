#pragma once
#include "global.hpp"

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
