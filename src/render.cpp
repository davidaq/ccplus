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
    const char* vshader;
    const char* fshader;
} programs[BLEND_MODE_COUNT];

void initGlobalVars() {
    static bool inited = false;
    if(inited)
        return;
    inited = true;

#define SET_PROGRAM(ID, NAME) programs[ID] = { \
        .name = "blend " #NAME, \
        .vshader = "shaders/fill.v.glsl", \
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
}


void CCPlus::mergeFrame(const GPUFrame& bottom, const GPUFrame& top, BlendMode blendmode) {
    initGlobalVars();
    if (bottom.width != top.width || 
        bottom.height != top.height) {
        log(logWARN) << "Merge frame requires frames to have equal sizes";
    }

    GLProgramManager* manager = GLProgramManager::getManager();
    GLuint program = (blendmode >= 0 && blendmode < BLEND_MODE_COUNT) ?
        manager->getProgram(
            programs[blendmode].name,
            programs[blendmode].vshader, 
            programs[blendmode].fshader) :
        manager->getProgram(
                "blend none",
                "shaders/fill.v.glsl",
                "shaders/blenders/none.f.glsl");
    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "tex_up"), 1);
    glUniform1i(glGetUniformLocation(program, "tex_down"), 2);

    // UP
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, top.textureID);

    // Bottom
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bottom.textureID);

    fillSprite();
}

void CCPlus::trackMatte(const GPUFrame& color, const GPUFrame& alpha, TrackMatteMode) {

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
