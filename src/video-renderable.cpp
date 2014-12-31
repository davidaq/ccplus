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
    isUserRes = uri[0] == 'x';
    path = parseUri2File(uri);

    VideoDecoder decoder(path, audioOnly ? VideoDecoder::DECODE_AUDIO : VideoDecoder::DECODE_AUDIO|VideoDecoder::DECODE_VIDEO);
    VideoInfo vinfo = decoder.getVideoInfo();
    duration = vinfo.duration;
    if(vinfo.hasVideoStream) {
        float d = duration - 1, t;
        if(d < 0)
            d = 0;
        decoder.seekTo(d);
        while(0 <= (t = decoder.decodeImage())) {
            d = t;
        }
        d += vinfo.frameTime + 1.0 / frameRate;
        t = duration - d;
        if(t < 0)
            t = -t;
        if(t < 2) {
            duration = d;
        }
    } else {
        audioOnly = false;
    }
}

VideoRenderable::~VideoRenderable() {
    release();
}

void VideoRenderable::release() {
    framesCache.clear();
    frameRefer.clear();
    audios = std::vector<int16_t>();
}

GPUFrame VideoRenderable::getGPUFrame(float time) {
    int frameNum = time2frame(time);
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
                frame = framesCache[imageFrameNum].decompressed();
                frame.ext.audio = framesCache[frameNum].ext.audio;
            } else {
                frame = framesCache[frameNum].decompressed();
            }
            lastFrame = frame.toGPU(false);
        }
        lastFrameImageFrameNum = imageFrameNum;
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
    const static int audioBufferSize = 8;
    if(audioStartTime + 0.1 > start || audioEndTime - 0.1 < start + duration) {
        VideoDecoder decoder(path, VideoDecoder::DECODE_AUDIO);
        audios = std::vector<int16_t>();
        decoder.seekTo(start, false);
        audioStartTime = decoder.decodeAudio(audios, audioBufferSize);
        audioEndTime = audioStartTime + audioBufferSize;
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
        VideoDecoder decoder(path, audioOnly ? VideoDecoder::DECODE_AUDIO : VideoDecoder::DECODE_AUDIO|VideoDecoder::DECODE_VIDEO);
        decoder.seekTo(partBeginTime - 1);
        float t = 0;
        while(t >= 0 && fnum - 1 > (t = decoder.decodeImage() * frameRate));

        int lastImageFrame = -1;
        for(; fnum <= partEnd; fnum++) {
            if(framesCache.count(fnum))
                continue;
            Frame cframe;
            if(t >= 0 && (lastImageFrame < 0 || t < fnum + 1)) {
                cframe = decoder.getDecodedImage();
                cframe.toNearestPOT(512, renderMode == PREVIEW_MODE);
#ifdef __ANDROID__
                if(!cframe.image.empty())
                    cv::cvtColor(cframe.image, cframe.image, CV_BGRA2RGBA);
#endif
                while(t >= 0 && t < fnum + 1)
                    t = decoder.decodeImage() * frameRate;
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

