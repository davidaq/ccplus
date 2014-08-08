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
        for(int k = 0; k < 99999999; k++)
            for(int j = 0; j < 99999999; j++)
                for(int i = 0; i < 99999999; i++);
    }
    profile(sleep) {
        sleep(2);
    }
    profileFlush;
}
