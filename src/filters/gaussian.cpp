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
    
    int scale = 1;
    while (size > 47) {
        size /= 2;
        scale *= 2;
    }
    if (size % 2 == 0) size += 1;
    if (size < 7) {
        size = 7;
    }
    int direction = (int) parameters[1];

    GLProgramManager* manager = GLProgramManager::getManager();

    auto sample = [manager] (GPUFrame frame, float scale) {
        int width = std::max(1.0f, frame->width * scale);
        int height = std::max(1.0f, frame->height * scale);
        GPUFrame ret = GPUFrameCache::alloc(width, height);
        GLuint program = manager->getProgram(
                "sample",
                "shaders/fill.v.glsl",
                "shaders/filters/transform.f.glsl");
        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame->textureID);

        ret->bindFBO();
        fillSprite();
        return ret;
    };

    if (scale > 1) {
        frame = sample(frame, 1.0f / scale);
    }
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
    float kernel[12];
    float offset[12];
    //L() << halfKernel.size() << " " << ksize;
    for (int i = 0; i < ksize; i++) {
        kernel[i] = halfKernel[i * 2] + halfKernel[i * 2 + 1];
        offset[i] = i * 2.0f + halfKernel[i * 2 + 1] / kernel[i];
        //std::cout << kernel[i] << std::endl;
    }
    for (int i = ksize; i < 12; i++) {
        kernel[i] = 0;
        offset[i] = 0;
    }

    glUniform1i(glGetUniformLocation(program, "ksize"), ksize);
    glUniform1fv(glGetUniformLocation(program, "gWeights"), 12, kernel);
    glUniform1fv(glGetUniformLocation(program, "gOffsets"), 12, offset);


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
        if (scale > 1)
            tmp = sample(tmp, scale);
        tmp->ext = frame->ext;
        return tmp;
    }
    if (scale > 1)
        ret = sample(ret, scale);
    ret->ext = frame->ext;

    return ret;
}
