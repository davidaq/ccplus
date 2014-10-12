#include "glprogram-manager.hpp"
#include "platform.hpp"

using namespace CCPlus;

GLProgramManager* GLProgramManager::getManager() {
    static GLProgramManager program;
    return &program; 
}

void GLProgramManager::clean() {
    for (auto& kv : programPool) {
        glDeleteProgram(kv.second);
    }
    programPool.clear();
}

GLuint GLProgramManager::getProgram(
        const std::string& name,
        const std::string& vshaderPath,
        const std::string& fshaderPath) {

    if (programPool.count(name)) {
        return programPool[name];
    }

    GLuint vertex_shader, fragment_shader, program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    cv::Mat tmp1 = readAsset(vshaderPath);
    int vsz = tmp1.total();
    cv::Mat tmp2 = readAsset(fshaderPath);
    int fsz = tmp2.total();
    glShaderSource(vertex_shader, 1, 
            (const char**)&tmp1.data, &vsz);
    glShaderSource(fragment_shader, 1, 
            (const char**)&tmp2.data, &fsz);

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);

    auto printCompileError = [] (GLuint shader) {
        GLint logMaxSize, logLength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, 
                &logMaxSize );
        char* logMsg = new char[logMaxSize];
        glGetShaderInfoLog( shader, logMaxSize, 
                &logLength, logMsg );
        printf("error message: %s\n", logMsg);
        delete[] logMsg;
    };

    GLint compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printCompileError(vertex_shader);
        log(logFATAL) << "Can't compile vertex shader for " << name;
    }

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printCompileError(fragment_shader);
        log(logFATAL) << "Can't compile fragment shader for " << name;
    }

    program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glBindAttribLocation(program, ATTRIB_VERTEX_POSITION, "vertex_position");
    glLinkProgram(program);

    programPool[name] = program;
    return program;
}
