#include "audio-renderable.hpp"
#include "utils.hpp"
#include "video-decoder.hpp"

using namespace CCPlus;

AudioRenderable::AudioRenderable(Context* ctx, const std::string& uri) 
  : AnimatedRenderable (ctx) {
    decoder = new VideoDecoder(uri, VideoDecoder::DECODE_VIDEO);
}

AudioRenderable::~AudioRenderable() {
    delete decoder;
}

void AudioRenderable::renderPart(float start, float duration) {
}

int AudioRenderable::getWidth() const {
    return 0;
}

int AudioRenderable::getHeight() const {
    return 0;
}

