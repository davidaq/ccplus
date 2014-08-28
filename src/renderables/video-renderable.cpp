#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(Context* context, const std::string& _uri) :
    AnimatedRenderable(context),
    uri(_uri)
{
    std::string path = uri;
    if (stringStartsWith(path, "file://")) 
        path = path.substr(7);
    path = generatePath(context->getInputDir(), path);
    std::string alpha_file = path + ".opacity.mp4";
    FILE* testFp = fopen(alpha_file.c_str(), "r");
    if(testFp) {
        fclose(testFp);
        alpha_decoder = new VideoDecoder(alpha_file, VideoDecoder::DECODE_VIDEO);
        decoder = new VideoDecoder(path + ".mp4");
    } else {
        alpha_decoder = 0;
        decoder = new VideoDecoder(path);
    }
}

VideoRenderable::~VideoRenderable() {
    delete decoder;
    delete alpha_decoder;
}

const std::string& VideoRenderable::getName() const {
    return uri;
}

void VideoRenderable::renderPart(float start, float duration) {
    // Audio
    decoder->seekTo(start);
    std::vector<int16_t> audios = decoder->decodeAudio(duration);

    // Video
    decoder->seekTo(start);
    if(alpha_decoder)
        alpha_decoder->seekTo(start);
    float gap = -1;
    float pos = -1;
    int lastFrame = -1;

    auto makeup_frames = [&](int f, int last_f) {
        if (last_f == -1 || f - last_f <= 1) return;
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

    int startFrameNumber = getFrameNumber(start);
    auto subAudio = [&startFrameNumber, this] (
            const std::vector<int16_t>& apart, 
            int f) {
        int rela = f - startFrameNumber;
        int nsig = CCPlus::AUDIO_SAMPLE_RATE / this->context->getFPS();
        std::vector<int16_t> ret;
        for (int i = 0; i < nsig && i + nsig * rela < apart.size(); i++)
            ret.push_back(apart[i + nsig * rela]);
        return ret;
    };

    cv::Mat alpha;
    while((pos = decoder->decodeImage()) + 0.001 > 0) {
        if(gap < 0.001)
            gap = (pos - start) / 3;
        int f = getFrameNumber(pos);
        std::string fp = getFramePath(f);

        // Make up lost frames
        makeup_frames(f, lastFrame);
        
        // Assume alpha channel video is synchronized with color channel video
        if(alpha_decoder)
            alpha_decoder->decodeImage();
        if (!rendered.count(f)) {
            Frame ret = decoder->getDecodedImage();
            if(alpha_decoder) {
                Frame opac = alpha_decoder->getDecodedImage();
                unsigned char* opacData = opac.getImage().data;
                unsigned char* frameData = ret.getImage().data;
                if(opac.getWidth() == ret.getWidth() && opac.getHeight() == ret.getHeight()) {
                    for(int i = 3, c = opac.getImage().total() * 4; i < c; i += 4) {
                        frameData[i] = opacData[i - 1];
                    }
                }
            }
            ret.setAudio(subAudio(audios, f));
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
    makeup_frames(getFrameNumber(start + duration), lastFrame);

    if (lastFrame == -1) {
        // Audio only
        float inter = 1.0 / context->getFPS();
        for (float i = start; i <= start + duration + inter; i += inter) {
            int f = getFrameNumber(i);
            makeup_frames(f, lastFrame);
            if (!rendered.count(f)) {
                Frame ret(subAudio(audios, f));
                std::string fp = getFramePath(f);
                ret.write(fp, 75);
                rendered.insert(f);
                lastFrame = f;
            }
        }    
    }
}

float VideoRenderable::getDuration() const {
    return decoder->getVideoInfo().duration;
}

int VideoRenderable::getWidth() const {
    return decoder->getVideoInfo().rwidth;
}

int VideoRenderable::getHeight() const {
    return decoder->getVideoInfo().rheight;
}

bool VideoRenderable::still(float t1, float t2) {
    // Assume video are always changing
    return false;
}
