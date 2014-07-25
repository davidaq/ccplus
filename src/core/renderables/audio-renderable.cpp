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
    decoder->seekTo(start);
    decoder->decodeAudio(this->getPCMPath(), duration);
}

Image AudioRenderable::getFrame(float time) const {
    // Audio is invisible~
    return Image();
}

int AudioRenderable::getWidth() const {
    return 0;
}

int AudioRenderable::getHeight() const {
    return 0;
}

std::string AudioRenderable::getPCMPath() const {
    return generatePath(context->getStoragePath(), uuid + ".pcm");
}

