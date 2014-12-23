#include "global.hpp"
#include "ccplay.hpp"
#include "ccplus.hpp"
#include "parallel-executor.hpp"
#include "frame.hpp"
#include <time.h>
#include <queue>

using namespace CCPlus::CCPlay;
using namespace CCPlus;

struct BufferObj {
    Frame frame;
    int fid;
};

int BUFFER_DURATION = 2;
bool keepRunning = false;
int currentFrame = 0; // Latest frame that hasn't/has been showed
Lock buffer_lock;
pthread_t buffer_thread = 0;
pthread_t play_thread = 0;
std::queue<BufferObj*> buffer;

PlayerInterface playerInterface = 0;
ProgressInterface progressInterface = 0;

void CCPlus::CCPlay::play(const char* _zimDir, bool blocking) {
    play(0, _zimDir, blocking);
}

void CCPlus::CCPlay::play(int key, const char* _zimDir, bool blocking) {
    std::string zimDir(_zimDir);
    stop();
    keepRunning = true;
    while (!buffer.empty()) {
        buffer.pop();
    }
    buffer_thread = ParallelExecutor::runInNewThread([zimDir] () {
        int lastFrame = 0x7fffffff;
        while (keepRunning) {
            // Clean useless frame
            buffer_lock.lock();
            while (!buffer.empty() && buffer.front()->fid != currentFrame) {
                BufferObj* tmp = buffer.front();
                buffer.pop();
                delete tmp;
            }
            buffer_lock.unlock();

            // Make sure buffer is not too big
            if (buffer.size() > BUFFER_DURATION * getFrameRate()) {
                usleep(5000); // Sleep 5 msecs
                continue;
            }
            int targetFrame = buffer.empty() ? currentFrame : (buffer.back()->fid + 1);

            if (targetFrame > lastFrame) {
                usleep(5000);
                continue;
            }

            char buf[32];
            sprintf(buf, "%07d.zim", targetFrame);
            std::string fn = generatePath(zimDir, buf);
            sprintf(buf, "%07d.zim", targetFrame + 1);
            std::string next_fn = generatePath(zimDir, buf);
            std::string eov_fn = generatePath(zimDir, "eov.zim");

            // Check next frame to make sure targetframe is 
            bool flag = file_exists(eov_fn);
            if (file_exists(fn) && (flag || file_exists(next_fn))) {
                BufferObj* obj = new BufferObj();
                obj->fid = targetFrame;
                obj->frame.read(fn);

                if (obj->frame.eov) {
                    lastFrame = obj->fid;
                }

                buffer.push(obj);
            }

            usleep(5000); // Sleep 5 msecs
        }
        buffer_thread = 0;
    });
    play_thread = ParallelExecutor::runInNewThread([key] () {
        float playerTime = 0.0;
        currentFrame = 0;
        const int WAITING = 0;
        const int INITING = -1;
        const int PLAYING = 1;
        int status = INITING; // 0 -> waiting, 1 -> playing, -1 -> initing
        double beforeTime = getSystemTime();
        while (keepRunning) {
            if (status == PLAYING) {
                double now = getSystemTime();
                float d = now - beforeTime;
                float frameTime = 1.0 / getFrameRate();
                if(d > 1.5 * frameTime)
                    d = 1.5 * frameTime;
                float desiredTime = currentFrame * frameTime;
                if (playerTime + d >= desiredTime) {
                    beforeTime = now;
                    playerTime += d;
                    buffer_lock.lock();
                    if (buffer.size() > 0 && buffer.front()->fid == currentFrame) {
                        // Invoke callback
                        log(logINFO) << "Playing: " << playerTime;
                        Frame* buf = &buffer.front()->frame;
                        if (playerInterface) {
                            playerInterface(key, desiredTime, buf->image.data, 
                                    buf->image.cols, buf->image.rows,
                                    buf->ext.audio.data, buf->ext.audio.total() * 2, 1.0);
                        }
                        if (buf->eov) {
                            log(logINFO) << "DONE! ";
                            buffer_lock.unlock();
                            playerInterface(key, 0, 0, 0, 0, 0, 0, 0);
                            keepRunning = false;
                            break;
                        }
                        currentFrame++;
                    } else {
                        log(logINFO) << "Start warting: " << currentFrame;
                        status = WAITING;
                    }
                    buffer_lock.unlock();
                }
            } else if (status == INITING) {
                if (buffer.size() > BUFFER_DURATION * getFrameRate() / 3) {
                    status = PLAYING;
                }
            } else {
                if (buffer.size() >= BUFFER_DURATION * getFrameRate()) {
                    status = PLAYING;
                } else if (progressInterface) {
                    progressInterface(key, 100.0 * buffer.size() / (1.0 * BUFFER_DURATION * getFrameRate()));
                }
            }
            usleep(5000);
        }
        play_thread = 0;
    });
    if (blocking) {
        if (play_thread) {
            pthread_join(play_thread, NULL);
        }
        if (buffer_thread) {
            pthread_join(buffer_thread, NULL);
        }
    }
}

void CCPlus::CCPlay::stop() {
    keepRunning = false;
    if (play_thread) {
        pthread_join(play_thread, NULL);
    }
    if (buffer_thread) {
        pthread_join(buffer_thread, NULL);
    }
}

void CCPlus::CCPlay::setBufferDuration(int buffer_duration) {
    BUFFER_DURATION = buffer_duration;
}

void CCPlus::CCPlay::attachPlayerInterface(PlayerInterface interface) {
    playerInterface = interface;
}

void CCPlus::CCPlay::attachProgressInterface(ProgressInterface interface) {
    progressInterface = interface;
}

