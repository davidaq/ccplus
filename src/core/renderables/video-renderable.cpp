#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(Context* context, const std::string& uri) :
    AnimatedRenderable(context)
{
    std::string path = uri;
    if (stringStartsWith(path, "file://")) 
        path = path.substr(7);
    decoder = new VideoDecoder(path, VideoDecoder::DECODE_VIDEO);
}

VideoRenderable::~VideoRenderable() {
    delete decoder;
}

void VideoRenderable::renderPart(float start, float duration) {
    decoder->seekTo(start);
    float gap = -1;
    float pos = -1;
    int lastFrame = -1;
    while((pos = decoder->decodeImage()) + 0.001 > 0) {
        if(gap < 0.001)
            gap = (pos - start) / 3;
        int f = getFrameNumber(pos);
        std::string fp = getFramePath(f);

        // Make up lost frames
        if (lastFrame != -1 && f - lastFrame > 1) {
            Image lost(getFramePath(lastFrame));
            for (int j = 1; j + lastFrame < f; j++) {
                std::string fp = getFramePath(j + lastFrame);
                lost.write(fp);
            }
        }
        
        if (!rendered.count(f)) {
            Image ret = decoder->getDecodedImage();
            ret.write(fp);
            rendered.insert(f);
            lastFrame = f;
        }

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

