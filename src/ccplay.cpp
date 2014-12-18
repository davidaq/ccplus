#include "global.hpp"
#include "ccplay.hpp"
#include "parallel-executor.hpp"
#include "frame.hpp"
#include <queue>

using namespace CCPlus;

const int BUFFER_DURATION = 2;
bool keepRunning = false;
int currentFrame = 0; // Latest frame that haven't been showed

PlayerInterface playerInterface = 0;

struct BufferObj {
    Frame buf;
    int fid;
};

void CCPlus::play(const std::string& zimDir, int fps) {
    keepRunning = true;
    pthread_mutex_t buffer_lock;
    std::queue<BufferObj*> buffer;
    pthread_t buffer_thread = ParallelExecutor::runInNewThread([fps, &buffer, &buffer_lock] () {
        while (keepRunning) {
            // Clean useless frame
            pthread_mutex_lock(&buffer_lock);
            while (!buffer.empty() && buffer.front().fid != currentFrame) {
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
            int targetFrame = buffer.empty() ? currentFrame : (buffer.back().fid + 1);

            char buf[32];
            sprintf(buf, "%07d.zim", targetFrame);
            std::string fn = generatePath(zimDir, buf);
            sprintf(buf, "%07d.zim", targetFrame + 1);
            std::string next_fn = generatePath(zimDir, buf);
            std::string eof_fn = generatePath(zimDir, "eof.zim");

            // Check next frame to make sure targetframe is 
            if (file_exists(next_fn) || file_exists(eof_fn)) {
                BufferObj* obj = new BufferObj();
                obj->fid = targetFrame;
                obj->frame.read(fn);

                queue.push(obj);
            }

            usleep(10000); // Sleep 10 msecs
        }
    });
    pthread_t play_thread = ParallelExecutor::runInNewThread([fps, &buffer, &buffer_lock] () {
        float time = 0;
        const int WAITING = 0;
        const int INITING = -1;
        const int PLAYING = 1;
        int status = INITING; // 0 -> waiting, 1 -> playing, -1 -> initing
        while (keepRunning) {
            usleep(5000);
            if (status == PLAYING) {
                time += 0.005;
                float desiredTime = currentFrame * 1.0 / fps;
                if (time >= desiredTime) {
                    pthread_mutex_lock(&buffer_lock);
                    if (buffer.size() > 0 && buffer.front()->fid == currentFrame) {
                        // Invoke callback
                        L() << "Invoke callback time: " << time;
                    } else {
                        status = 0;
                    }
                    pthread_mutex_unlock(&buffer_lock);
                }
            } else {
                if (status == INITING) {
                    if (buffer.size() > 0) {
                        status = PLAYING;
                    }
                } else {
                    if (buffer.size() > BUFFER_DURATION * fps) {
                        status = PLAYING;
                    }
                }
            }
        }
    });
}

void CCPlus::stop() {
    keepRunning = false;
}

void CCPlus::rewind() {
    currentFrame = 0;
}

void CCPlus::attachPlayerInterface(PlayerInterface interface) {
    playerInterface = interface;
}

