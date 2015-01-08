#include "global.hpp"
#include "utils.hpp"
#include "video-decoder.hpp"

#include <cstdio>
#include <map>

using namespace CCPlus;

/*
 * Followed document in http://www.media.mit.edu/pia/Research/deepview/exif.html
 *
 */

int CCPlus::getImageRotation(const std::string& jpgpath) {
    FILE* f = fopen(jpgpath.c_str(), "rb");   
    int ret = -1;
    if (f == NULL) return ret; 
    char tmp[64];

    auto consume = [](FILE* f, int n) {
        fseek(f, n, SEEK_CUR);
    };

    auto nread = [](FILE* f, char* tmp, int origin, int offset) {
        for (int i = 0; i < offset; i++)  
            if (fscanf(f, "%c", &tmp[origin + i]) != 1) 
                throw std::ios_base::failure("Unrecgonized file format");
    };

    auto bytesToInt = [](const char* s, int n) {
        int ret = 0;
        for (int i = 0; i < n; i++) 
            ret = ret * 256 + s[i];
        return ret;
    };

    std::map<int, int> formatBytes = {
        {1, 1}, {2, 1}, {3, 2}, {4, 4}, {5, 8}, {6, 1},
        {7, 1}, {8, 2}, {9, 4}, {10, 8}, {11, 4}, {12, 8}
    };

    std::map<int, int> retTable = {
        {-1, 0}, {1, 0}, {3, 180}, {8, 270}, {6, 90}
    };

    enum State {
        START = 0, EXIF, TIFF, IFD, ORIENTATION, DONE
    };

    State state = START;
    
    // Assume 2 bytes reading is OK 
    while (state != DONE && (fscanf(f, "%c%c", &tmp[0], &tmp[1]) > 1)) {
        if (state == START) {
            // Found ffe1 -> app1 marker !!
            if ((unsigned char) tmp[0] == 0xff && (unsigned char) tmp[1] == 0xe1) {
                state = EXIF;
                continue;
            }
        } else if (state == EXIF) {
            if (!strncmp(tmp, "Ex", 2)) {
                nread(f, tmp, 2, 2);
                if (!strncmp(tmp, "Exif", 4)) {
                    state = TIFF;
                    // Eat empty bytes
                    consume(f, 2);
                    continue;
                }
            }
        } else if (state == TIFF) {
            // GOT TFIF reader
            if (!strncmp(tmp, "MM", 2)) {
                state = IFD;

                // EAT tag mask
                consume(f, 2);

                nread(f, tmp, 0, 4);

                int skip = bytesToInt(tmp, 4) - 8;
                consume(f, skip);
                continue;
            } else if (!strncmp(tmp, "II", 2)) {
                // DAMN the Intel
                log(logWARN) << "Image is using Intel type align, it unsupported now";
                return -1;
            }

        } else if (state == IFD) {
            int nifd = bytesToInt(tmp, 2);
            for (int i = 0; i < nifd; i++) {
                // Read tag number
                nread(f, tmp, 0, 2);
                // Not rotation
                if (bytesToInt(tmp, 2) == 0x0112) 
                    state = ORIENTATION;   

                // Read format
                nread(f, tmp, 0, 2);
                // Must be a short -> 3
                int format = bytesToInt(tmp, 2);
                int sz = formatBytes[format];

                // Read component number -> must be 0x00000001
                nread(f, tmp, 0, 4);
                
                int ncomp = bytesToInt(tmp, 4);

                // If total bytes more than 4, then skip
                int totalb = sz * ncomp;
                if (totalb > 4) {
                    consume(f, 4);
                    continue;
                }

                // Read data
                for (int j = 0; j < ncomp; j++)
                    nread(f, tmp, 0, sz);
                
                if (state == ORIENTATION) {
                    ret = bytesToInt(tmp, 2);
                    state = DONE;
                    break;
                }
            }
            if (state == IFD) {
                // Only have one chance
                return -1;
            }
        }
        fseek(f, -1, SEEK_CUR);
    }

    fclose(f);
    // CW
    return retTable[ret];
}

cv::Mat CCPlus::readAsset(const char* _name) {
    std::string name = generatePath(assetsPath, _name);
    FILE* fp = fopen(name.c_str(), "rb");
    if(!fp) {
        log(logWARN) << "Asset " + name + " not found";
        return cv::Mat();
    }
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    cv::Mat ret(1, len, CV_8U);
    fread(ret.data, 1, len, fp);
    fclose(fp);
    return ret;
}

static inline bool hasAudio(const std::vector<int16_t>& data) {
    if(data.empty())
        return false;
    int total = 0;
    for(int i = 1; i < data.size(); i++) {
        int d = data[i] - data[i - 1];
        if(d < 0)
            total -= d;
        else
            total += d;
    }
    total /= data.size();
    return total > 30;
}

bool CCPlus::hasAudio(const std::string& uri, float start, float duration) {
    VideoDecoder decoder(uri, VideoDecoder::DECODE_AUDIO);
    const float vduration = decoder.getVideoInfo().duration;
    if(start + duration > vduration) {
        start = 0;
        duration = vduration;
    }
    if(duration < 0.5) {
        decoder.seekTo(start);
        return ::hasAudio(decoder.decodeAudio(duration));
    }
    for(float t = 0; t < duration; t += 1.5) {
        decoder.seekTo(start + t);
        if(::hasAudio(decoder.decodeAudio(0.3))) {
            return true;
        }
    }
    return false;
}

bool CCPlus::checkGLError() {
    int status = glGetError();
    bool flag = status ? true : false;
    while (status) {
        switch(status) {
            case 0:
                break;
                return false;
            default:
                log(logERROR) << "gl error: " << status;
                break;
        }
        status = glGetError();
    }
    return flag;
}

bool CCPlus::checkGLFramebuffer() {
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != 0x8cd5) {
       log(logERROR) << "Framebuffer status:" << status; 
    }
    return false;
}

bool CCPlus::isGLFramebufferComplete() {
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }
    return true;
}
