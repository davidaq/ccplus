#include "gpu-worker.hpp"

using namespace CCPlus;

GPUWorker::GPUWorker() {

}

void GPUWorker::loadShader(const std::string& vertexShader, 
        const std::string& fragmentShader) {

}

void GPUWorker::initPipeline(std::function<void(void)> f) {
    f();
}

void GPUWorker::run(Frame& f) {
    
}
