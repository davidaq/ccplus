#include <string>
#include "platform.hpp"
#include <QGLPixelBuffer>

using namespace CCPlus;


void* CCPlus::createGLContext() {
    QGLFormat format;
    format.setVersion(2, 1);
    format.setAlpha(true);
    format.setDepth(false);
    format.setStencil(false);
    format.setDoubleBuffer(false);
    QGLPixelBuffer* glBuffer = new QGLPixelBuffer(1024, 1024, format, 0);
    glBuffer->makeCurrent();
    return glBuffer;
}

void CCPlus::destroyGLContext(void* ptr) {
    QGLPixelBuffer* glBuffer = (QGLPixelBuffer*)ptr;
    glBuffer->doneCurrent();
    delete glBuffer;
}

void CCPlus::copyAssets() {
}
