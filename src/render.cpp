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

struct {
    const char* name;
    const char* fshader;
} programs[BLEND_MODE_COUNT + TRKMTE_MODE_COUNT];

void initGlobalVars() {
    static bool inited = false;
    if(inited)
        return;
    inited = true;

#define SET_PROGRAM(ID, NAME) programs[ID] = { \
        .name = "blend " #NAME, \
        .fshader = "shaders/blenders/" #NAME ".f.glsl" };
    SET_PROGRAM(DEFAULT, default);
    SET_PROGRAM(ADD, add);
    SET_PROGRAM(MULTIPLY, multiply);
    SET_PROGRAM(SCREEN, screen);
    SET_PROGRAM(DISOLVE, disolve);
    SET_PROGRAM(DARKEN, darken);
    SET_PROGRAM(LIGHTEN, lighten);
    SET_PROGRAM(OVERLAY, overlay);
    SET_PROGRAM(DIFFERENCE, difference);
#undef SET_PROGRAM
#define SET_PROGRAM(ID, NAME) programs[TRKMTE_ ## ID + BLEND_MODE_COUNT - 1] = { \
        .name = "trkMat " #NAME, \
        .fshader = "shaders/trkmat/" #NAME ".f.glsl" };
    SET_PROGRAM(ALPHA, alpha);
    SET_PROGRAM(ALPHA_INV, alpha_inv);
    SET_PROGRAM(LUMA, luma);
    SET_PROGRAM(LUMA_INV, luma_inv);
#undef SET_PROGRAM
}

cv::Mat mergeAudio(cv::Mat base, cv::Mat in) {
    if(in.empty()) {
        return base.clone();
    }
    if(base.empty()) {
        return in.clone();
    }
    cv::Mat ret = cv::Mat(1, AUDIO_SAMPLE_RATE / Context::getContext()->fps, CV_16S, cv::Scalar(0));
    cv::Mat o;
    if(base.total() != in.total()) {
        cv::resize(in, o, base.size(), CV_INTER_LINEAR);
    } else {
        o = in;
    }
    int16_t* basePtr = base.ptr<int16_t>(0);
    int16_t* inPtr = o.ptr<int16_t>(0);
    int16_t* retPtr = ret.ptr<int16_t>(0);
    for(int i = 0; i < base.total(); i++) {
        retPtr[i] = basePtr[i] + inPtr[i];
    }
    return ret;
}

GPUFrame blendUsingProgram(GLuint program, const GPUFrame& bottom, const GPUFrame& top) {
    if (!bottom) return top;
    if (!top) return bottom;
    if ((bottom->width != top->width || bottom->height != top->height)) {
        if (bottom->textureID && top->textureID) {
            log(logWARN) << "Merge frame requires frames to have equal sizes";
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

    glUniform1i(glGetUniformLocation(program, "tex_up"), 1);
    glUniform1i(glGetUniformLocation(program, "tex_down"), 2);

    // UP
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, top->textureID);

    // Bottom
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bottom->textureID);

    GPUFrame frame = GPUFrameCache::alloc(top->width, top->height);
    fillSprite();
    return frame;
}

GPUFrame CCPlus::mergeFrame(GPUFrame bottom, GPUFrame top, BlendMode blendmode) {
    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = (blendmode >= 0 && blendmode < BLEND_MODE_COUNT) ?
        manager->getProgram(
            programs[blendmode].name,
            "shaders/fill.v.glsl",
            programs[blendmode].fshader) :
        manager->getProgram(
                "blend none",
                "shaders/fill.v.glsl",
                "shaders/blenders/none.f.glsl");
    return blendUsingProgram(program, bottom, top);
}

GPUFrame CCPlus::trackMatte(GPUFrame color, GPUFrame alpha, TrackMatteMode mode) {
    GLuint program = GLProgramManager::getManager()->getProgram(
        programs[mode + BLEND_MODE_COUNT - 1].name,
        "shaders/fill.v.glsl",
        programs[mode + BLEND_MODE_COUNT - 1].fshader);
    return blendUsingProgram(program, alpha, color);
}

void CCPlus::fillSprite() {
    initGlobalVars();
    if(!Context::getContext()->flags.count("init fill sprite")) {
        glGenBuffers(1, &squareVBO);
        glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareCoord), squareCoord, GL_STATIC_DRAW);
        Context::getContext()->flags.insert("init fill sprite");
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    }
    glEnableVertexAttribArray(ATTRIB_VERTEX_POSITION);
    glVertexAttribPointer(ATTRIB_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void CCPlus::fillTriangles(const std::vector<std::pair<float, float>>& pnts) {
    if (pnts.size() % 3 != 0 || pnts.size() == 0) {
        log(logERROR) << "Invalid fillTriangles parameters number";
        return;
    } 

    int sz = pnts.size() * 2;
    float* fv = new float[sz];
    for (int i = 0; i < sz; i+=2) {
        fv[i] = pnts[i / 2].first;
        fv[i + 1] = pnts[i / 2].second;
    }

    //for (int i = 0; i < sz; i++)
    //    std::cout << fv[i] << std::endl;

    //if (!Context::getContext()->flags.count("init fill trg")) {
    //    glGenBuffers(1, &trianglesVBO);
    //    glBindBuffer(GL_ARRAY_BUFFER, trianglesVBO);
    //    Context::getContext()->flags.insert("init fill trg");
    //} else {
    //    glBindBuffer(GL_ARRAY_BUFFER, trianglesVBO);
    //}

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(ATTRIB_VERTEX_POSITION);
    glVertexAttribPointer(ATTRIB_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 0, fv);
    glDrawArrays(GL_TRIANGLES, 0, sz / 2);
    delete [] fv;
}
