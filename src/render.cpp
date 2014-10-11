#include "render.hpp"
#include "context.hpp"
#include "gpu-frame.hpp"
#include "glprogram-manager.hpp"

using namespace CCPlus;

GLuint squareVBO;
float squareCoord[8];
int squareIndex[4];

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
    squareCoord[0] = -1.f; squareCoord[1] = -1.f;
    squareCoord[2] = 1.f; squareCoord[3] = -1.f;
    squareCoord[4] = 1.f; squareCoord[5] = 1.f;
    squareCoord[6] = -1.f; squareCoord[7] = 1.f;

    squareIndex[0] = 0;
    squareIndex[1] = 1;
    squareIndex[2] = 2;
    squareIndex[3] = 3;

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


void CCPlus::mergeFrame(GPUFrame& bottom, GPUFrame& top, BlendMode blendmode) {
    initGlobalVars();
    bottom.bindFBO();
    glBindTexture(GL_TEXTURE_2D, top.textureID);
    GLuint program = GLProgramManager::getManager()->getProgram(
            programs[blendmode].name, programs[blendmode].vshader,
            programs[blendmode].fshader);
    glUseProgram(program);
    fillSprite();
}

void CCPlus::trackMatte(GPUFrame& color, GPUFrame& alpha, TrackMatteMode) {

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
    glDrawElements(GL_TRIANGLE_STRIP, 2, GL_UNSIGNED_INT, squareIndex);
}

