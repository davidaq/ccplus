#include "render.hpp"
#include "context.hpp"
#include "gpu-frame.hpp"
#include "glprogram-manager.hpp"

using namespace CCPlus;

GLuint squareVBO;
GLuint trianglesVBO;
float squareCoord[8] = {
    1.0,  1.0,  
    -1.0, 1.0,  
    1.0,  -1.0, 
    -1.0, -1.0 
};

void CCPlus::initGL() {
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glGenBuffers(1, &squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareCoord), squareCoord, GL_STATIC_DRAW);

    GLProgramManager::getManager()->init();
}

cv::Mat mergeAudio(cv::Mat base, cv::Mat in) {
    if(in.empty()) {
        return base.clone();
    }
    if(base.empty()) {
        return in.clone();
    }
    cv::Mat ret = cv::Mat(1, audioSampleRate / frameRate, CV_16S, cv::Scalar(0));
    cv::Mat o;
    if(base.total() != in.total()) {
        L();
        cv::resize(in, o, base.size(), CV_INTER_LINEAR);
    } else {
        o = in;
    }
    int16_t* basePtr = base.ptr<int16_t>(0);
    int16_t* inPtr = o.ptr<int16_t>(0);
    int16_t* retPtr = ret.ptr<int16_t>(0);
    for(int i = 0; i < base.total(); i++) {
        int res = basePtr[i] + inPtr[i];
        if(res > 32767)
            res = 32767;
        if(res < -32768)
            res = -32768;
        retPtr[i] = res;
    }
    return ret;
}

GPUFrame CCPlus::blendUsingProgram(GLuint program, const GPUFrame& bottom, const GPUFrame& top) {
    if (!bottom) return top;
    if (!top) return bottom;
    if ((bottom->width != top->width || bottom->height != top->height)) {
        if (bottom->textureID && top->textureID) {
            log(logWARN) << "Merge frame requires frames to have equal sizes: " << 
                bottom->width << "*" << bottom->height << " -- " <<
                top->width << "*" << top->height;
            return bottom;
        }
        if (bottom->textureID) {
            bottom->ext.audio = mergeAudio(bottom->ext.audio, top->ext.audio);
            return bottom;
        } else {
            top->ext.audio = mergeAudio(bottom->ext.audio, top->ext.audio);
            return top;
        }
    }

    glUseProgram(program);

    GPUFrame frame = GPUFrameCache::alloc(top->width, top->height);
    frame->bindFBO(false);

    frame->ext.audio = mergeAudio(bottom->ext.audio, top->ext.audio);

    // UP
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, top->textureID);

    // Bottom
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bottom->textureID);

    fillSprite();
    return frame;
}

GPUFrame CCPlus::shaderBlend(GPUFrame bottom, GPUFrame top, BlendMode blendmode) {
    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = (blendmode >= 0 && blendmode < BLEND_MODE_COUNT) ?
        manager->getProgram((GLProgram)(blend_default + blendmode - DEFAULT))
        : manager->getProgram(blend_none);
    return blendUsingProgram(program, bottom, top);
}

GPUFrame CCPlus::trackMatte(GPUFrame color, GPUFrame alpha, TrackMatteMode mode) {
    if (!alpha) {
        alpha = GPUFrameCache::alloc(color->width, color->height);
        glClearColor(0, 0, 0, 0);
        alpha->bindFBO();
    }
    GLuint program = GLProgramManager::getManager()->getProgram((GLProgram)(trkmte_alpha + mode - TRKMTE_ALPHA));
    return blendUsingProgram(program, color, alpha);
}

void CCPlus::fillSprite() {
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glEnableVertexAttribArray(ATTRIB_VERTEX_POSITION);
    glVertexAttribPointer(ATTRIB_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void CCPlus::fillTriangles(const std::vector<std::pair<float, float>>& pnts) {
    if (pnts.size() % 3 != 0 || pnts.size() == 0) {
        log(logERROR) << "Invalid fillTriangles parameters number: " << pnts.size();
        return;
    } 

    int sz = pnts.size() * 2;
    float* fv = new float[sz];
    for (int i = 0; i < sz; i+=2) {
        fv[i] = pnts[i / 2].first;
        fv[i + 1] = pnts[i / 2].second;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(ATTRIB_VERTEX_POSITION);
    glVertexAttribPointer(ATTRIB_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 0, fv);
    glDrawArrays(GL_TRIANGLES, 0, sz / 2);
    delete [] fv;
}

typedef void (*VoidFunc)(void);
#define UNDEFINED 0xffffff
struct BlendMethod {
    GLenum srcColor = UNDEFINED;
    GLenum dstColor = UNDEFINED;
    GLenum srcAlpha = UNDEFINED;
    GLenum dstAlpha = UNDEFINED;
};

BlendMethod *blendMethods = 0;

void CCPlus::blend(GPUFrame& dst, const GPUFrame& texture, BlendMode blendMode) {
    if(!texture)
        return;
    // setup blend functions
    if(!blendMethods) {
        blendMethods = new BlendMethod[BLEND_MODE_COUNT];
        blendMethods[NONE] = BlendMethod {
            .srcColor = GL_ONE,
            .dstColor = GL_ZERO,
        };
        blendMethods[DEFAULT] = BlendMethod {
            .srcColor = GL_ONE,
            .dstColor = GL_ONE_MINUS_SRC_ALPHA,
        };
        blendMethods[ADD] = BlendMethod {
            .srcColor = GL_ONE,
            .srcAlpha = GL_ONE,
            .dstColor = GL_ONE,
            .dstAlpha = GL_ONE_MINUS_SRC_ALPHA,
        };
        blendMethods[SCREEN] = BlendMethod {
            .srcColor = GL_ONE,
            .dstColor = GL_ONE_MINUS_SRC_COLOR,
        };
    }
    
    BlendMethod& method = blendMethods[blendMode];
    if(method.srcColor == UNDEFINED) {
        dst = shaderBlend(dst, texture, blendMode);
    } else {
        GLuint program = GLProgramManager::getManager()->getProgram(plainfill);
        glUseProgram(program);
        glEnable(GL_BLEND);
        if(method.srcAlpha == UNDEFINED) {
            glBlendFunc(method.srcColor, method.dstColor);
        } else {
            glBlendFuncSeparate(method.srcColor, method.dstColor, method.srcAlpha, method.dstAlpha);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->textureID);
        fillSprite();
        glDisable(GL_BLEND);
    }
}
