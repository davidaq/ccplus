#include "render.hpp"
#include "context.hpp"
#include "gpu-frame.hpp"
#include "glprogram-manager.hpp"

using namespace CCPlus;

GLuint squareVBO;
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

