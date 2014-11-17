#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"
#include "gpu-frame.hpp"
#include "profile.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(const std::string& _uri, bool _audioOnly) :
    uri(_uri),
    audioOnly(_audioOnly)
{
    std::string path = parseUri2File(uri);
    alpha_decoder = 0;
    if(audioOnly) {
        decoder = new VideoDecoder(path);
    } else {
        std::string alpha_file = path + ".opacity.mp4";
        FILE* testFp = fopen(alpha_file.c_str(), "r");
        if(testFp) {
            fclose(testFp);
            alpha_decoder = new VideoDecoder(alpha_file, VideoDecoder::DECODE_VIDEO);
            decoder = new VideoDecoder(path + ".mp4");
        } else {
            decoder = new VideoDecoder(path, VideoDecoder::DECODE_AUDIO|VideoDecoder::DECODE_VIDEO);
        }
    }
}

VideoRenderable::~VideoRenderable() {
    release();
}

void VideoRenderable::release() {
    framesCache.clear();
    frameRefer.clear();
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
    if (prepared) {
        return;
    }
    prepared = true;
    for(const auto& part : usedFragments)
        preparePart((int)(part.first - 0.5), (int)(part.second - part.first + 1));
}

GPUFrame VideoRenderable::getGPUFrame(float time) {
    int frameNum = time2frame(time);
    for(;frameRefer.count(frameNum);) {
        frameNum = frameRefer[frameNum];
    }
    if(framesCache.count(frameNum)) {
        return framesCache[frameNum].toGPU();
    } else {
        return GPUFrame();
    }
}

void VideoRenderable::preparePart(float start, float duration) {
    profile(videoDecode) {
        // Audio
        decoder->seekTo(start);
        std::vector<int16_t> audios = decoder->decodeAudio(duration);

        float gap = -1;
        float pos = -1;
        int lastFrame = -1;

        auto makeup_frames = [&](int f, int last_f) {
            if(!framesCache.count(last_f)) return;
            if (last_f == -1 || f - last_f <= 1) return;
            for (int j = 1; j + last_f < f; j++) {
                int insf = j + last_f;
                if(!framesCache.count(insf)) {
                    frameRefer[insf] = last_f;
                }
            }
        };

        int startFrameNumber = time2frame(start);
        auto subAudio = [&startFrameNumber, this] (
                const std::vector<int16_t>& apart, 
                int f) {
            int rela = f - startFrameNumber;
            int nsig = audioSampleRate / frameRate;
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

        // Video
        if(!audioOnly) {
            decoder->seekTo(start);
            if(alpha_decoder)
                alpha_decoder->seekTo(start);
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

                    ret.toNearestPOT(renderMode == PREVIEW_MODE ? 256 : 512);
                    //if(!alpha_decoder) {
                    //    uint8_t* ptr = ret.image.data;
                    //    for(int i = 0; i < ret.image.total(); i++) {
                    //        const uint8_t &a = ptr[3];
                    //        if(a == 0 || a == 255) {
                    //            ptr += 4;
                    //            continue;
                    //        }
                    //        *ptr *= *(ptr++) * 255 / a;
                    //        *ptr *= *(ptr++) * 255 / a;
                    //        *ptr *= *(ptr++) * 255 / a;
                    //        ptr++;
                    //    }
                    //}
                    if(ret.image.cols * ret.image.rows > 100000)
                        framesCache[f] = ret.compressed();
                    else
                        framesCache[f] = ret;
                    lastFrame = f;
                }

                if(pos - start + gap > duration) {
                    break;
                }
            }
        }
        if (lastFrame == -1) {
            // Audio only
            float inter = 1.0 / frameRate;
            for (float i = start; i <= start + duration + inter; i += inter) {
                int f = time2frame(i);
                makeup_frames(f, lastFrame);
                if (!framesCache.count(f)) {
                    Frame ret;
                    ret.ext.audio = subAudio(audios, f);
                    framesCache[f]= ret;
                    lastFrame = f;
                }
            }    
        }

        // Make up some missed frame :
        // Used while decoding low fps video
        makeup_frames(time2frame(start + duration), lastFrame);
    }
}

int VideoRenderable::time2frame(float time) {
    return (int)(time * frameRate);
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
        duration = max * 1.0f / frameRate;
        float oduration = decoder->getVideoInfo().duration;
        if(duration < oduration && oduration - duration < oduration * 0.1)
            duration = oduration;
    }
    return decoder->getVideoInfo().duration;
}

