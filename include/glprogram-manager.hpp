#pragma once
#include "global.hpp"

namespace CCPlus {
    enum AttribLocation {
        ATTRIB_VERTEX_POSITION = 0,
    };
    
    enum GLProgram {
#define CCPLUS_PROGRAM(NAME, ...) NAME,
#include "program-list.hpp"
#undef CCPLUS_PROGRAM
        GL_PROGRAM_COUNT
    };
}

class CCPlus::GLProgramManager : public Object {
public:
    static CCPlus::GLProgramManager* getManager();
    void init();
    void clean();

    // @ Returns the opengl program id of the program, passing uniform locations through U0~9 following the
    // uniform chache order defined in program-list.hpp
    GLuint getProgram(GLProgram programName, GLuint* U0 = 0, GLuint* U1 = 0, GLuint* U2 = 0, GLuint* U3 = 0,
            GLuint* U4 = 0, GLuint* U5 = 0, GLuint* U6 = 0, GLuint* U7 = 0, GLuint* U8 = 0, GLuint* U9 = 0);

private: 
    typedef std::function<void(GLuint, std::vector<GLuint>& )> SetupFunc;
    struct ProgramInfo {
        const char* name;
        const char* vShader;
        const char* fShader;
        GLuint programID;
        GLuint uniforms[10];
        SetupFunc setupFunc;
    } programInfo[GL_PROGRAM_COUNT];
    void registerProgram(const char* name, GLProgram program, const char* vShader, const char* fShader, SetupFunc setupFunc);

    std::map<std::string, GLuint> compiledObject;
    GLuint compileShader(const std::string& shaderSourcePath, GLuint shaderType);
};

