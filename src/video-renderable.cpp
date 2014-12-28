#include "video-renderable.hpp"
#include "context.hpp"
#include "video-decoder.hpp"
#include "utils.hpp"
#include "gpu-frame.hpp"
#include "profile.hpp"
#include "ccplus.hpp"

using namespace CCPlus;

VideoRenderable::VideoRenderable(const std::string& uri, bool _audioOnly) :
    audioOnly(_audioOnly)
{
    if(uri[0] == 'x')
        useSlowerCompress = true;
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
    if(decoder)
        duration = decoder->getVideoInfo().duration;
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

GPUFrame VideoRenderable::getGPUFrame(float time) {
    int frameNum = time2frame(time);
    if(lastFrame && frameNum == lastFrameNum)
        return lastFrame;
    if(!framesCache.count(frameNum))
        lastFrame = GPUFrame();
    else {
        Frame frame;
        if(frameRefer.count(frameNum)) {
            frame = framesCache[frameRefer[frameNum]].decompressed();
            frame.ext.audio = framesCache[frameNum].ext.audio;
        } else {
            frame = framesCache[frameNum].decompressed();
        }
        lastFrame = frame.toGPU(false);
    }
    lastFrameNum = frameNum;
    return lastFrame;
}

void VideoRenderable::releasePart(float start, float duration) {
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
    int fnum = start * frameRate;
    int fend = (start + duration) * frameRate + 1;
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
        decoder->seekTo(partBeginTime);
        std::vector<int16_t> audios = decoder->decodeAudio((partEnd - fnum + 1) * 1.0 / frameRate);
        decoder->seekTo(partBeginTime);
        float t = decoder->decodeImage() * frameRate;
        int lastImageFrame = -1;
        for(; fnum <= partEnd; fnum++) {
            Frame cframe;
            if(t >= 0) {
                if(lastImageFrame < 0 || t < fnum + 1) {
                    cframe = decoder->getDecodedImage();
                    cframe.toNearestPOT(512, renderMode == PREVIEW_MODE);
#ifdef __ANDROID__
                        if(!cframe.image.empty())
                            cv::cvtColor(cframe.image, cframe.image, CV_BGRA2RGBA);
#endif
                    while(t >= 0 && t < fnum + 1)
                        t = decoder->decodeImage() * frameRate;
                    lastImageFrame = fnum;
                } else {
                    frameRefer[fnum] = lastImageFrame;
                }
            }
            if(!audios.empty()) {
                int audioFrameEnd = ((fnum + 1) * 1.0 / frameRate - partBeginTime) * audioSampleRate;
                if(audios.size() > audioFrameEnd) {
                    int audioFrameStart = (fnum * 1.0 / frameRate - partBeginTime) * audioSampleRate;
                    cframe.ext.audio = cv::Mat(1, audioFrameEnd - audioFrameStart, CV_16S);
                    memcpy(cframe.ext.audio.data, &audios[audioFrameStart], (audioFrameEnd - audioFrameStart) * 2);
                }
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

