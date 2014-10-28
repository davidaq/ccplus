#include "glprogram-manager.hpp"
#include "platform.hpp"

using namespace CCPlus;

GLProgramManager* GLProgramManager::getManager() {
    static GLProgramManager program;
    return &program; 
}

void GLProgramManager::init() {
    clean();
    for(int i = 0; i < GL_PROGRAM_COUNT; i++) {
        programInfo[i].programID = 0;
    }
    static bool inited = false;
    if(inited)
        return;
    inited = true;
#define CCPLUS_PROGRAM(NAME, VSHADER, FSHADER, FUNC) registerProgram(#NAME, NAME, VSHADER, FSHADER, FUNC);
#include "program-list.hpp"
#undef CCPLUS_PROGRAM
}

void GLProgramManager::registerProgram(const char* name, GLProgram program, const char* vShader, const char* fShader, SetupFunc setupFunc) {
    programInfo[program].name = name;
    programInfo[program].vShader = vShader;
    programInfo[program].fShader = fShader;
    programInfo[program].setupFunc = setupFunc;
}

void GLProgramManager::clean() {
    compiledObject.clear();
}

GLuint GLProgramManager::compileShader(const std::string& shaderSourcePath, GLuint shaderType) {
    if(compiledObject.count(shaderSourcePath))
        return compiledObject[shaderSourcePath];
    GLuint shaderId = glCreateShader(shaderType);
    compiledObject[shaderSourcePath] = shaderId;
    std::string shaderSource = readTextAsset(shaderSourcePath);
#ifdef GLSLES
#define GLSLVERSION "#version 100\n#define VERSION_100\n"
#else
#define GLSLVERSION "#version 120\n#define VERSION_120\n"
#endif
    if(GL_VERTEX_SHADER == shaderType) {
        shaderSource = GLSLVERSION "#define IS_VERTEX_SHADER\n#include <global.glsl>\n" + shaderSource;
    } else {
        shaderSource = GLSLVERSION "#define IS_FRAGMENT_SHADER\n#include <global.glsl>\n" + shaderSource;
    }
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
        return 0;
    }
    return shaderId;
}

GLuint GLProgramManager::getProgram(GLProgram programName, GLuint* U0, GLuint* U1, GLuint* U2, GLuint* U3,
            GLuint* U4, GLuint* U5, GLuint* U6, GLuint* U7, GLuint* U8, GLuint* U9) {
    ProgramInfo& programInfo = this->programInfo[programName];
    if(!programInfo.programID) {
        GLuint vertex_shader = compileShader(programInfo.vShader, GL_VERTEX_SHADER);
        GLuint fragment_shader = compileShader(programInfo.fShader, GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();

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
            log(logFATAL) << "Program linked failed for " << programInfo.name;
            exit(0);
        }
        programInfo.programID = program;

        if(programInfo.setupFunc) {
            std::vector<GLuint> uniforms;
            glUseProgram(program);
            programInfo.setupFunc(programInfo.programID, uniforms);
            for(int i = 0; i < 10 && i < uniforms.size(); i++) {
                programInfo.uniforms[i] = uniforms[i];
            }
        }
    }

    if(U0) *U0 = programInfo.uniforms[0];
    if(U1) *U1 = programInfo.uniforms[1];
    if(U2) *U2 = programInfo.uniforms[2];
    if(U3) *U3 = programInfo.uniforms[3];
    if(U4) *U4 = programInfo.uniforms[4];
    if(U5) *U5 = programInfo.uniforms[5];
    if(U6) *U6 = programInfo.uniforms[6];
    if(U7) *U7 = programInfo.uniforms[7];
    if(U8) *U8 = programInfo.uniforms[8];
    if(U9) *U9 = programInfo.uniforms[9];
    
    return programInfo.programID;
}
