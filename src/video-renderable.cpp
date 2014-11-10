#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"
#include "gpu-frame.hpp"
#include "profile.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(const std::string& _uri) :
    uri(_uri)
{
    std::string path = parseUri2File(uri);
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
    release();
}

void VideoRenderable::release() {
    framesCache.clear();
    if(decoder) {
        delete decoder;
        decoder = 0;
    }
    if(alpha_decoder) {
        delete alpha_decoder;
        alpha_decoder = 0;
    }
}

void VideoRenderable::prepare() {
    for(const auto& part : usedFragments)
        preparePart((int)(part.first - 0.5), (int)(part.second - part.first + 1));
}

GPUFrame VideoRenderable::getGPUFrame(float time) {
    int frameNum = time2frame(time);
    if(framesCache.count(frameNum)) {
        const Frame& frame = framesCache[frameNum];
        GPUFrame ret = GPUFrameCache::alloc(frame.image.cols, frame.image.rows);
        ret->load(frame);
        return ret;
    } else {
        return GPUFrame();
    }
}

void VideoRenderable::preparePart(float start, float duration) {
    profile(videoDecode) {
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
            if(!framesCache.count(last_f)) return;
            if (last_f == -1 || f - last_f <= 1) return;
            for (int j = 1; j + last_f < f; j++) {
                int insf = j + last_f;
                if(!framesCache.count(insf)) {
                    framesCache[insf] = framesCache[last_f];
                }
            }
        };

        int startFrameNumber = time2frame(start);
        auto subAudio = [&startFrameNumber, this] (
                const std::vector<int16_t>& apart, 
                int f) {
            int rela = f - startFrameNumber;
            int nsig = AUDIO_SAMPLE_RATE / Context::getContext()->fps;
            int pos = nsig * rela;
            int size = std::min<int>(nsig, apart.size() - pos);
            if(size > 0) {
                cv::Mat ret(1, size, CV_16S);
                memcpy(ret.data, ((const int16_t*)(&apart[0]) + pos), size * 2);
                return ret;
            } else {
                return cv::Mat();
            }
        };

        cv::Mat alpha;
        while((pos = decoder->decodeImage()) + 0.001 > 0) {
            if(gap < 0.001)
                gap = (pos - start) / 3;
            int f = time2frame(pos);

            // Make up lost frames
            makeup_frames(f, lastFrame);
            
            // Assume alpha channel video is synchronized with color channel video
            if(alpha_decoder)
                alpha_decoder->decodeImage();
            if (!framesCache.count(f)) {
                Frame ret = decoder->getDecodedImage();
                if(alpha_decoder) {
                    Frame opac = alpha_decoder->getDecodedImage();
                    unsigned char* opacData = opac.image.data;
                    unsigned char* frameData = ret.image.data;
                    if(opac.image.cols == ret.image.cols &&
                            opac.image.rows == ret.image.rows) {
                        for(int i = 3, c = opac.image.total() * 4; i < c; i += 4) {
                            frameData[i] = opacData[i - 1];
                        }
                    }
                }
                ret.ext.audio = subAudio(audios, f);
#ifdef __ANDROID__
                if(!ret.image.empty())
                    cv::cvtColor(ret.image, ret.image, CV_BGRA2RGBA);
#endif
                cv::resize(ret.image, ret.image, cv::Size(ret.image.cols / 2, ret.image.rows / 2));
                ret.ext.scaleAdjustX = 2;
                ret.ext.scaleAdjustY = 2;
                framesCache[f] = ret;
                lastFrame = f;
            }

            if(pos - start + gap > duration) {
                break;
            }
        }

        // Make up some missed frame :
        // Used while rendering low fps video
        makeup_frames(time2frame(start + duration), lastFrame);

        if (lastFrame == -1) {
            // Audio only
            float inter = 1.0 / Context::getContext()->fps;
            for (float i = start; i <= start + duration + inter; i += inter) {
                int f = time2frame(i);
                makeup_frames(f, lastFrame);
                if (!framesCache.count(f)) {
                    Frame ret;
                    ret.ext.audio = subAudio(audios, f);
                    framesCache[f] = ret;
                    lastFrame = f;
                }
            }    
        }
    }
}

int VideoRenderable::time2frame(float time) {
    return (int)(time * Context::getContext()->fps);
}

float VideoRenderable::getDuration() {
    if(duration > 0)
        return duration;
    if(!framesCache.empty()) {
        int max = 0;
        for(const auto& item : framesCache) {
            if(item.first > max)
                max = item.first;
        }
        max++;
        duration = max * 1.0f / Context::getContext()->fps;
        float oduration = decoder->getVideoInfo().duration;
        if(duration < oduration && oduration - duration < oduration * 0.1)
            duration = oduration;
    }
    return decoder->getVideoInfo().duration;
}

