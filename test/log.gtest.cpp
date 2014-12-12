#include "gtest/gtest.h"

#include "global.hpp"
#include "unistd.h"

using namespace CCPlus;

TEST(Log, TestAll) {
    log(logERROR) << "It's error!";
    log(logWARN) << "It's warn!";
    log(logINFO) << "It's info!";
    log(logDEBUG) << "It's debug!";

    //log(logFATAL) << "its fatal, program will exist here";
}

TEST(Log, Profile) {
    profile(three_loops) {
        
    }
    profile(sleep) {
        sleep(2);
    }
    profile(sleep) {
        sleep(1);
    }

    profileBegin(Hello);
        sleep(1);
    profileEnd(Hello);
    profileFlush;
}
