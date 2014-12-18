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
    std::queue<BufferObj> buffer;
    pthread_t buffer_thread = ParallelExecutor::runInNewThread([&buffer, &buffer_lock] () {
        while (true) {
            // Clean 
            pthread_mutex_lock(&buffer_lock);
            while (!buffer.empty() && buffer.front().fid != currentFrame) {
                buffer.pop();
            }
            pthread_mutex_unlock(&buffer_lock);

            //int targetFrame = buffer.empty() ? 
            usleep(500000);
        }
    });
    pthread_t play_thread = ParallelExecutor::runInNewThread([&buffer, &buffer_lock] () {
        // Clean 
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

