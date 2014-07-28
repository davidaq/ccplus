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
    auto makeup_frames = [&](int f, int last_f, auto decoder) {
        if (last_f == -1 || f - last_f <= 1) return;
        //Image lost = decoder->getDecodedImage();
        Frame lost = Frame(getFramePath(last_f));
        for (int j = 1; j + last_f < f; j++) {
            int insf = j + last_f;
            if(!rendered.count(insf)) {
                std::string fp = getFramePath(insf);
                lost.write(fp, 75);
                rendered.insert(insf);
            }
        }
    };
    while((pos = decoder->decodeImage()) + 0.001 > 0) {
        if(gap < 0.001)
            gap = (pos - start) / 3;
        int f = getFrameNumber(pos);
        std::string fp = getFramePath(f);

        // Make up lost frames
        makeup_frames(f, lastFrame, decoder);
        
        if (!rendered.count(f)) {
            Frame ret = decoder->getDecodedImage();
            ret.write(fp, 75);
            rendered.insert(f);
            lastFrame = f;
        }

        if(pos - start + gap > duration) {
            break;
        }
    }

    // Make up some missed frame :
    // Used while rendering low fps video
    makeup_frames(getFrameNumber(start + duration), lastFrame, decoder);
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
