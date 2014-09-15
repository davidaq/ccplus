#include <string>

#include "gpu-worker.hpp"
#include "frame.hpp"
#include "logger.hpp"

using namespace CCPlus;

GPUWorker::GPUWorker() {

}

void GPUWorker::loadShader(const char* vertexShaderStr, 
        const char* fragmentShaderStr) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, 
            &vertexShaderStr, NULL);
    glShaderSource(fragmentShader, 1, 
            &fragmentShaderStr, NULL);
    
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    auto printError = [] (GLuint shader) {
        GLint logMaxSize, logLength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logMaxSize  );
        char* logMsg = new char[logMaxSize];
        glGetShaderInfoLog( shader, logMaxSize, &logLength, logMsg  );
        log(logDEBUG) << std::string(logMsg);
        delete[] logMsg;
    };

    GLint compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printError(vertexShader);
        log(logFATAL) << "Vertex shader failed to compile";
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printError(fragmentShader);
        log(logFATAL) << "Fragment shader failed to compile";
    }

    // TODO: whether re-create program
    this->program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glUseProgram(program);

    // TODO: docs recommend to detach as soon as possible
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
}

void GPUWorker::loadShader(const std::string& vertexShader, 
        const std::string& fragmentShader) {
    loadShader(vertexShader.c_str(), fragmentShader.c_str());
}

void GPUWorker::run(std::function<void(GLuint)> func, Frame& f) {
    int width = f.getWidth();
    int height = f.getHeight();
    // Generate texture
    GLuint texture;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
            width, height, 0, GL_BGRA, 
            GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

    // Generate FBO
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // Bind texture to fbo
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_2D, texture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        printf("failed to make complete framebuffer object %x\n", status);
    }
    
    glViewport(0, 0, width, height);
    glClearColor(0.5, 0.5, 0.5, 0.9);
    glClear(GL_COLOR_BUFFER_BIT);
    func(this->program);
    glFinish();

    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    GLubyte pixels[width * height * 4];
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

    memcpy(f.getImage().data, pixels, width * height * 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
