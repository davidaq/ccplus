#pragma once

#include <cstring>
#include <functional>

// OpenGL TODO: check OS
#include <OpenGL/opengl.h>
#include <OpenGl/gl.h>

namespace CCPlus {
    class GPUWorker;
    class Frame;
}

class CCPlus::GPUWorker {

public:
    GPUWorker();
    
    /*
     * Load and lazy-compile shaders
     */
    void loadShader(const std::string& vertexShaderStr, 
            const std::string& fragmentShaderStr);
    void loadShader(const char* vertexShaderStr, 
            const char* fragmentShaderStr);

    /*
     * Put output in @f after rendering
     */
    void run(std::function<void(GLuint)>, CCPlus::Frame& f);

private:
    GLuint program = 0;
};
