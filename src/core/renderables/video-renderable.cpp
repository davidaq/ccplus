#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(Context* context, const std::string& uri) :
    AnimatedRenderable(context),
    decoder(new VideoDecoder(uri, VideoDecoder::DECODE_VIDEO))
{
}

VideoRenderable::~VideoRenderable() {
    delete decoder;
}

void VideoRenderable::renderPart(float start, float duration) {
    decoder->seekTo(start);
    float gap = -1;
    float pos = -1;
    while((pos = decoder->decodeImage()) > 0) {
        if(gap < 0.001)
            gap = (pos - start) / 3;
        int f = getFrameNumber(pos);
        std::string fp = getFramePath(f);
        
        Image ret = decoder->getDecodedImage();
        ret.write(fp);
        rendered.insert(f);

        if(pos - start + gap > duration) {
            break;
        }
    }
}

float VideoRenderable::getDuration() const {
    return decoder->getVideoInfo().duration;
}

int VideoRenderable::getWidth() const {
    return decoder->getVideoInfo().width;
}

int VideoRenderable::getHeight() const {
    return decoder->getVideoInfo().height;
}

