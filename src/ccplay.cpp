#include "global.hpp"
#include "ccplay.hpp"
#include "parallel-executor.hpp"
#include "frame.hpp"
#include <queue>

using namespace CCPlus;

struct BufferObj {
    Frame frame;
    int fid;
};

const int BUFFER_DURATION = 2;
bool keepRunning = false;
int currentFrame = 0; // Latest frame that haven't been showed
float playerTime = 0;
pthread_mutex_t buffer_lock;
std::queue<BufferObj*> buffer;

PlayerInterface playerInterface = 0;
ProgressInterface progressInterface = 0;

void CCPlus::play(const std::string& zimDir, int fps, bool blocking) {
    keepRunning = true;
    pthread_t buffer_thread = ParallelExecutor::runInNewThread([&zimDir, fps] () {
        bool finished = false;
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
            if (finished) {
                usleep(10000); // Sleep 10 msecs
                continue;
            }
            int targetFrame = buffer.empty() ? currentFrame : (buffer.back()->fid + 1);

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
                    finished = true;
                }

                buffer.push(obj);
            }

            usleep(10000); // Sleep 10 msecs
        }
    });
    pthread_t play_thread = ParallelExecutor::runInNewThread([fps] () {
        playerTime = 0.0;
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
                            return;
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
    });

    if (blocking) {
        pthread_join(play_thread, NULL);
    }
}

void CCPlus::stop() {
    keepRunning = false;
}

void CCPlus::rewind() {
    currentFrame = 0;
    playerTime = 0.0;
}

void CCPlus::attachPlayerInterface(PlayerInterface interface) {
    playerInterface = interface;
}

void CCPlus::attachProgressInterface(ProgressInterface interface) {
    progressInterface = interface;
}
