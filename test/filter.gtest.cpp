#include "gtest/gtest.h" 

#include "filter.hpp"

using namespace CCPlus;

CCPLUS_FILTER(ExampleX) {
    printf("Hello, this is from example filter\n");
}

TEST(Filter, Example) {
    Filter testExample("Example");
    std::vector<float> empty;
    testExample.apply(0, 0, empty);
}