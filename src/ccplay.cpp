#include "global.hpp"
#include "ccplay.hpp"
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
pthread_mutex_t buffer_lock;
pthread_t buffer_thread = 0;
pthread_t play_thread = 0;
std::queue<BufferObj*> buffer;

PlayerInterface playerInterface = 0;
ProgressInterface progressInterface = 0;

void CCPlus::CCPlay::play(const std::string& zimDir, int fps, bool blocking) {
    stop();
    keepRunning = true;
    while (!buffer.empty()) {
        buffer.pop();
    }
    buffer_thread = ParallelExecutor::runInNewThread([&zimDir, fps] () {
        int lastFrame = 0x7fffffff;
        while (keepRunning) {
            // Clean useless frame
            pthread_mutex_lock(&buffer_lock);
            while (!buffer.empty() && buffer.front()->fid != currentFrame) {
                BufferObj* tmp = buffer.front();
                buffer.pop();
                delete tmp;
            }
            pthread_mutex_unlock(&buffer_lock);

            // Make sure buffer is not too big
            if (buffer.size() > BUFFER_DURATION * fps) {
                usleep(10000); // Sleep 10 msecs
                continue;
            }
            int targetFrame = buffer.empty() ? currentFrame : (buffer.back()->fid + 1);

            if (targetFrame > lastFrame) {
                usleep(10000);
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

            usleep(10000); // Sleep 10 msecs
        }
        buffer_thread = 0;
    });
    play_thread = ParallelExecutor::runInNewThread([fps] () {
        float playerTime = 0.0;
        currentFrame = 0;
        const int WAITING = 0;
        const int INITING = -1;
        const int PLAYING = 1;
        int status = INITING; // 0 -> waiting, 1 -> playing, -1 -> initing
        while (keepRunning) {
            usleep(5000);
            if (status == PLAYING) {
                playerTime += 0.005;
                float desiredTime = currentFrame * 1.0 / fps;
                if (playerTime >= desiredTime) {
                    pthread_mutex_lock(&buffer_lock);
                    if (buffer.size() > 0 && buffer.front()->fid == currentFrame) {
                        // Invoke callback
                        L() << "Playing: " << playerTime;
                        Frame* buf = &buffer.front()->frame;
                        if (playerInterface) {
                            playerInterface(desiredTime, buf->image.data, buf->image.cols, 
                                buf->image.rows, buf->ext.audio.data, buf->ext.audio.total(), 1.0);
                        }
                        if (buf->eov) {
                            L() << "DONE! ";
                            pthread_mutex_unlock(&buffer_lock);
                            keepRunning = false;
                            break;
                        }
                        currentFrame++;
                    } else {
                        L() << "Start warting: " << currentFrame;
                        status = WAITING;
                    }
                    pthread_mutex_unlock(&buffer_lock);
                }
            } else {
                if (status == INITING) {
                    if (buffer.size() > 0) {
                        status = PLAYING;
                    }
                } else {
                    if (buffer.size() >= BUFFER_DURATION * fps) {
                        status = PLAYING;
                    } else {
                        if (progressInterface) {
                            progressInterface(100.0 * buffer.size() / (1.0 * BUFFER_DURATION * fps));
                        }
                    }
                }
            }
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

