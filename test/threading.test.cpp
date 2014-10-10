#include "gtest/gtest.h"
#include "parallel-executor.hpp"

using namespace CCPlus;

TEST(Threading, SpawnThreads) {
    Semaphore o;
    int a = 0;
    ParallelExecutor::runInNewThread([&a,&o]() {
        while(a < 9999999) {
            o.wait();
            sleep(1);
            a*=1.5;
        }
    });
    L() << a;
    ParallelExecutor::runInNewThread([&a,&o]() {
        o.wait();
        a++;
    });
    ParallelExecutor::runInNewThread([&a,&o]() {
        o.wait();
        a++;
    });
    L() << a;

    while(a < 999999999) {
        sleep(1);
        o.notify();
        L() << a;
    }
}
