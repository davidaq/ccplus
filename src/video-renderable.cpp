#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"
#include "gpu-frame.hpp"
#include "profile.hpp"
#include "ccplus.hpp"

namespace CCPlus {
    namespace VideoRenderableFlags {
        const int AUDIO  = 1 << 1;
        const int VIDEO  = 1 << 2;
        const int STATIC = 1 << 3;
    };
};
using namespace CCPlus;
using namespace CCPlus::VideoRenderableFlags;

VideoRenderable::VideoRenderable(const std::string& uri, bool audioOnly) {
    isUserRes = uri[0] == 'x';
    path = parseUri2File(uri);

    IVideoDecoderRef decoder = openDecoder(path, audioOnly ? VideoDecoder::DECODE_AUDIO : VideoDecoder::DECODE_AUDIO|VideoDecoder::DECODE_VIDEO, isUserRes);
    VideoInfo vinfo = decoder->getVideoInfo();
    duration = vinfo.duration;

    // probe file
    float t1 = decoder->decodeImage(), t2 = decoder->decodeImage();
    if(t1 > -10 && t2 > -10 && t2 > t1) {
        flags |= VIDEO;
    }
    if(!decoder->decodeAudio(2).empty()) {
        flags |= AUDIO;
    }
    if(!flags && t1 > -10) {
        flags |= STATIC;
    }
}

VideoRenderable::~VideoRenderable() {
    release();
}

void VideoRenderable::release() {
    framesCache.clear();
    frameRefer.clear();
    audios = std::vector<int16_t>();
    lastFrame = GPUFrame();
    lastFrameNum = 0;
    lastFrameImageFrameNum = -1;
    decoder = 0;
}

GPUFrame VideoRenderable::getGPUFrame(float time) {
    if(!flags)
        return GPUFrame();
    int frameNum = time2frame(time);
    if(flags & STATIC) {
        frameNum = 0;
    }
    if(lastFrame && frameNum == lastFrameNum)
        return lastFrame;
    if(!framesCache.count(frameNum)) {
        return GPUFrame();
    } else {
        Frame frame;
        int imageFrameNum = frameNum;
        while(frameRefer.count(imageFrameNum)) {
            imageFrameNum = frameRefer[imageFrameNum];
        }
        if(lastFrame && lastFrameImageFrameNum > 0 && lastFrameImageFrameNum == imageFrameNum) {
            lastFrame->ext.audio = framesCache[frameNum].ext.audio;
        } else {
            if(imageFrameNum != frameNum) {
                frame = framesCache[imageFrameNum];
                frame.ext.audio = framesCache[frameNum].ext.audio;
            } else {
                frame = framesCache[frameNum];
            }
            lastFrame = frame.toGPU(false);
        }
        lastFrameImageFrameNum = imageFrameNum;
    }
    lastFrameNum = frameNum;
    return lastFrame;
}

void VideoRenderable::releasePart(float start, float duration) {
    if(flags & STATIC) {
        Renderable::releasePart(start, duration);
        return;
    }
    int fnum = start * frameRate;
    int fend = (start + duration) * frameRate + 1;
    for(; fnum <= fend; fnum++) {
        if(frameCounter.count(fnum)) {
            frameCounter[fnum]--;
            if(frameCounter[fnum] <= 0) {
                frameRefer.erase(fnum);
                framesCache.erase(fnum);
                frameCounter.erase(fnum);
            }
        }
    }
}

void VideoRenderable::preparePart(float start, float duration) {
    if(flags & STATIC) {
        Renderable::preparePart(start, duration);
        IVideoDecoderRef decoder = openDecoder(path, VideoDecoder::DECODE_VIDEO, isUserRes);
        decoder->decodeImage();
        framesCache[0] = decoder->getDecodedImage();
        return;
    }
    const static int audioBufferSize = 8;
    if(flags & AUDIO && (audioStartTime + 0.1 > start || audioEndTime - 0.1 < start + duration)) {
        IVideoDecoderRef decoder = openDecoder(path, VideoDecoder::DECODE_AUDIO, isUserRes);
        audios = std::vector<int16_t>();
        decoder->seekTo(start, false);
        audioStartTime = decoder->decodeAudio(audios, audioBufferSize);
        audioEndTime = audioStartTime + audioBufferSize;
        if(this->duration > 2) {
            if(audioStartTime < 0.5) {
                int from = 0;
                int to = (0.5 - audioStartTime) * audioSampleRate;
                for(int i = from; i <= to && i < audios.size(); i++) {
                    audios[i] *= (i - from) * 2.0 / audioSampleRate;
                }
            }
            if(audioEndTime > this->duration - 0.5) {
                int from = (this->duration - 0.5 - audioStartTime) * audioSampleRate;
                int to = (this->duration - audioStartTime) * audioSampleRate;
                for(int i = from; i <= to && i < audios.size(); i++) {
                    audios[i] *= (to - i) * 2.0 / audioSampleRate;
                }
            }
        }
    }
    int fnum = start * frameRate;
    if(fnum > 0)
        fnum--;
    int fend = (start + duration) * frameRate + 2;
    while(fnum <= fend) {
        if(framesCache.count(fnum)) {
            frameCounter[fnum]++;
            fnum++;
            continue;
        }
        int partEnd = fnum + 1;
        while(partEnd < fend && !framesCache.count(partEnd))
            partEnd++;
        float partBeginTime = fnum * 1.0 / frameRate;
        float t = -100;
        if(flags & VIDEO) {
            profile(decodeImageINIT) {
                if(decoder) {
                    int d = decoderTime * frameRate - fnum;
                    if(d < 0)
                        d = -d;
                    if(d > 1) {
                        decoder = 0;
                    } else {
                        t = decoderTime;
                    }
                }
                if(!decoder) {
                    decoder = openDecoder(path, VideoDecoder::DECODE_VIDEO, isUserRes);
                    if(fnum > 1) {
                        decoder->seekTo(partBeginTime - 0.1);
                    }
                    do {
                        t = decoder->decodeImage();
                    } while(t > -10 && fnum - 0.01 > t * frameRate);
                    decoderTime = t;
                }
            }
        }

        int lastImageFrame = -1;
        int maxsz = 512;
        if(renderMode == PREVIEW_MODE) {
            maxsz = isUserRes ? 200 : 300;
        }
        for(; fnum <= partEnd; fnum++) {
            if(framesCache.count(fnum))
                continue;
            Frame cframe;
            if(t > -10 && (lastImageFrame < 0 || t * frameRate < fnum + 1)) {
                profile(getDecodedImage) {
                    cframe = decoder->getDecodedImage(maxsz);
                }
#ifdef __ANDROID__
                if(!cframe.image.empty())
                    cv::cvtColor(cframe.image, cframe.image, CV_BGRA2RGBA);
#endif
                profile(decodeImage)
                while(t > -10 && t * frameRate < fnum + (renderMode == PREVIEW_MODE ? 1.4 : 1))
                    t = decoder->decodeImage();
                decoderTime = t;
                lastImageFrame = fnum;
            } else if(lastImageFrame >= 0) {
                frameRefer[fnum] = lastImageFrame;
            }
            if(!audios.empty()) {
                int audioFrameSZ = audioSampleRate / frameRate;
                int audioFrameStart = (fnum * 1.0 / frameRate - audioStartTime) * audioSampleRate;
                if(audioFrameStart < 0)
                    audioFrameStart = 0;
                int audioFrameEnd = audioFrameStart + audioFrameSZ;
                if(audios.size() >= audioFrameEnd) {
                    cframe.ext.audio = cv::Mat(1, audioFrameSZ, CV_16S);
                    memcpy(cframe.ext.audio.data, &audios[audioFrameStart], audioFrameSZ * 2);
                }
                audioFrameStart = audioFrameEnd;
            }
            framesCache[fnum] = cframe.compressed();
            frameCounter[fnum]++;
        }
    }
}

int VideoRenderable::time2frame(float time) {
    return (int)(time * frameRate + 0.5);
}

float VideoRenderable::getDuration() {
    return duration;
}

