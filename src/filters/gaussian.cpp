#include "filter.hpp"
#include "glprogram-manager.hpp"
#include "gpu-frame-cache.hpp"
#include "gpu-frame-impl.hpp"
#include "render.hpp"
#include "externals/triangulate.h"

using namespace cv;
using namespace CCPlus;

inline std::vector<float> GenerateSeparableGaussKernel(int kernelSize) {
    int halfKernelSize = kernelSize / 2;

    // Come from OpenCV
    float sigma = 0.3 * ((kernelSize - 1) * 0.5 - 1) + 0.8;

    std::vector<float> kernel;
    kernel.resize(kernelSize);

    const float cPI= 3.14159265358979323846;
    float mean     = halfKernelSize;
    float sum      = 0.0;
    for (int x = 0; x < kernelSize; ++x) 
    {
        kernel[x] = (float)sqrt( exp( -0.5 * (pow((x-mean)/sigma, 2.0) + pow((mean)/sigma,2.0)) )
            / (2 * cPI * sigma * sigma) );
        sum += kernel[x];
    }
    for (int x = 0; x < kernelSize; ++x) 
        kernel[x] /= (float)sum;

    return kernel;
}

CCPLUS_FILTER(gaussian) {
    if (parameters.size() < 2) {
        log(logERROR) << "In sufficient parameters for gaussian filter";
        return frame;
    }
    int size = (int) parameters[0];
    if (size % 2 == 0) size += 1;
    size = 7;
    if (size > 99) {
        log(logWARN) << "Currently gaussian filter only support kernel size between 7 ~ 99, got: " << size;
        size = 99;
    }
    if (size < 7) {
        log(logWARN) << "Currently gaussian filter only support kernel size between 7 ~ 99, got: " << size;
        size = 7;
    }
    int direction = (int) parameters[1];

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = manager->getProgram(
            "filter_gaussian",
            "shaders/filters/gaussian.v.glsl",
            "shaders/filters/gaussian.f.glsl"
            );
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // Divide by 4 for acceleration

    // Get gaussian kernel
    std::vector<float> fullKernel = GenerateSeparableGaussKernel(size);
    std::vector<float> halfKernel;
    for (int i = size / 2; i >= 0; i--)
        halfKernel.push_back(fullKernel[i]);
    halfKernel[0] *= 0.5f; // Center half

    int ksize = halfKernel.size() / 2;
    //TODO: cache
    float* kernel = new float[ksize];
    float* offset = new float[ksize];
    //L() << halfKernel.size() << " " << ksize;
    for (int i = 0; i < ksize; i++) {
        kernel[i] = halfKernel[i * 2] + halfKernel[i * 2 + 1];
        offset[i] = i * 2.0f + halfKernel[i * 2 + 1] / kernel[i];
        //std::cout << kernel[i] << std::endl;
    }


    glUniform1i(glGetUniformLocation(program, "ksize"), ksize);
    glUniform1fv(glGetUniformLocation(program, "gWeights"), ksize, kernel);
    glUniform1fv(glGetUniformLocation(program, "gOffsets"), ksize, offset);

    delete [] kernel;
    delete [] offset;

    GPUFrame ret = GPUFrameCache::alloc(frame->width, frame->height);
    GPUFrame tmp = GPUFrameCache::alloc(frame->width, frame->height);
    if (direction != 2) { // With X
        tmp->bindFBO();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);
        glUniform2f(glGetUniformLocation(program, "pixelOffset"), 1.0f / frame->width, 0);
        fillSprite(); // Draw on tmp
    } 
    if (direction != 3) { // With Y
        glUniform2f(glGetUniformLocation(program, "pixelOffset"), 0, 1.0f / frame->width);
        ret->bindFBO();
        if (direction == 2) { // never go x
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, frame->textureID);
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tmp->textureID);
        }
        fillSprite();
    } else {
        tmp->ext = frame->ext;
        return tmp;
    }
    ret->ext = frame->ext;
    return ret;
}
