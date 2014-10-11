#include "gtest/gtest.h"
#include "parallel-executor.hpp"

using namespace CCPlus;

TEST(Threading, SpawnThreads) {
    Semaphore o;
    int a = 0;
    ParallelExecutor::runInNewThread([&a,&o]() {
        o.wait();
        a++;
    });
    ParallelExecutor::runInNewThread([&a,&o]() {
        o.wait();
        a++;
        o.wait();
        a++;
    });

    EXPECT_EQ(0, a);
    o.notify();
    EXPECT_TRUE(a > 0);
}
