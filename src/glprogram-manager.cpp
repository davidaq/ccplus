#include "glprogram-manager.hpp"
#include "platform.hpp"
#include "render.hpp"

using namespace CCPlus;

GLProgramManager* GLProgramManager::getManager() {
    static GLProgramManager program;
    return &program; 
}

void GLProgramManager::clean() {
    programPool[0].clear();
    programPool[1].clear();
}

bool compileShader(const std::string& shaderSourcePath, GLuint& shaderId, bool isVertexShader) {
    std::string shaderSource = readTextAsset(shaderSourcePath);
#ifdef GLSLES
#define GLSLVERSION "#version 100\n#define VERSION_100\n"
#else
#define GLSLVERSION "#version 120\n#define VERSION_120\n"
#endif
    if(isVertexShader)
        shaderSource = GLSLVERSION "#define IS_VERTEX_SHADER\n#include <global.glsl>\n" + shaderSource;
    else
        shaderSource = GLSLVERSION "#define IS_FRAGMENT_SHADER\n#include <global.glsl>\n" + shaderSource;
    // process includes
    size_t pos;
    while(std::string::npos != (pos = shaderSource.find("#include"))) {
        size_t f = shaderSource.find('<', pos);
        size_t l = shaderSource.find('>', pos);
        std::string includeFile = shaderSource.substr(f + 1, l - f - 1);
        shaderSource.replace(pos, l - pos + 1, readTextAsset("shaders/" + includeFile));
    }
    const char* shaderSourceData = shaderSource.c_str();
    int sz = shaderSource.size();
    glShaderSource(shaderId, 1, &shaderSourceData, &sz);
    glCompileShader(shaderId);
    GLint compiled;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logMaxSize, logLength;
        glGetShaderiv( shaderId, GL_INFO_LOG_LENGTH, 
                &logMaxSize );
        char* logMsg = new char[logMaxSize];
        glGetShaderInfoLog( shaderId, logMaxSize, 
                &logLength, logMsg );
        log(logERROR) << "Can't compile shader for " << shaderSourcePath << ':' << logMsg;
        delete[] logMsg;
        log(logFATAL) << shaderSource;
        exit(0);
        return false;
    }
    return true;
}

GLuint GLProgramManager::getProgram(
        const std::string& name,
        const std::string& vshaderPath,
        const std::string& fshaderPath) {
    
    auto& programPool = this->programPool[currentRenderThread()];

    if (programPool.count(name)) {
        return programPool[name];
    }

    GLuint vertex_shader, fragment_shader, program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vshaderPath, vertex_shader, true);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fshaderPath, fragment_shader, false);

    program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glBindAttribLocation(program, ATTRIB_VERTEX_POSITION, "vertex_position");
    glLinkProgram(program);
    GLint compiled;
    glGetProgramiv(program, GL_LINK_STATUS, &compiled);
    if (!compiled)  {
        GLint logMaxSize, logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logMaxSize);
        char* logMsg = new char[logMaxSize];
        glGetProgramInfoLog(program, logMaxSize, 
                &logLength, logMsg);
        printf("error message: %s\n", logMsg);
        delete[] logMsg;
        log(logFATAL) << "Program linked failed for " << name;
        exit(0);
    }

    programPool[name] = program;
    return program;
}
