#include "gtest/gtest.h"

#include "global.hpp"

using namespace CCPlus;

TEST(Log, TestAll) {
    log(logERROR) << "It's error!";
    log(logWARN) << "It's warn!";
    log(logINFO) << "It's info!";
    log(logDEBUG) << "It's debug!";

    //log(logFATAL) << "its fatal, program will exist here";
}
