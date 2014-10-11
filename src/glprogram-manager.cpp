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

    GLuint vertex_shader, fragment_shader;
    GLuint program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    cv::Mat tmp1 = readAsset(vshaderPath);
    glShaderSource(vertex_shader, 1, (const char**)&tmp1.data, NULL);
    cv::Mat tmp2 = readAsset(fshaderPath);
    glShaderSource(fragment_shader, 1, (const char**)&tmp2.data, NULL);

    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);

    GLint compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        log(logFATAL) << "Can't compile vertex shader for " << name;
    }

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        log(logFATAL) << "Can't compile fragment shader for " << name;
    }

    program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    programPool[name] = program;
    return program;
}
