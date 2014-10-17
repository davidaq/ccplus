#include "glprogram-manager.hpp"
#include "platform.hpp"

using namespace CCPlus;

GLProgramManager* GLProgramManager::getManager() {
    static GLProgramManager program;
    return &program; 
}

void GLProgramManager::clean() {
    programPool.clear();
}

bool compileShader(const std::string& shaderSourcePath, GLuint& shaderId, bool isVertexShader) {
    cv::Mat tmpData = readAsset(shaderSourcePath.c_str());
    std::string shaderSource = std::string((char*)tmpData.data, (char*)tmpData.data + tmpData.total());
#ifdef GLSLES
    if(isVertexShader)
        shaderSource = shaderSource.replace(0, 12, "#version 100\nprecision mediump float;\n");
    else
        shaderSource = shaderSource.replace(0, 12, "#version 100\nprecision lowp float;\n");
#endif
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
        printf("error message: %s\n", logMsg);
        delete[] logMsg;
        log(logFATAL) << "Can't compile shader for " << shaderSourcePath;
            exit(0);
        return false;
    }
    return true;
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
