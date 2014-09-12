#pragma once

#include <cstring>
#include <functional>

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
    void loadShader(const std::string& vertexShader, 
            const std::string& fragmentShader);

    /*
     * Init vertexs
     */
    void initPipeline(std::function<void(void)> f);

    /*
     * Put output in @f after rendering
     */
    void run(CCPlus::Frame& f);
};
