#include "gtest/gtest.h" 

#include "filter.hpp"

using namespace CCPlus;

TEST(Filter, Example) {
    // This test currently asserts nothing, passes if no segment fault occurs
    printf("This test should print PASS on the next line\n");
    std::vector<float> empty;
    Image src("test/res/test.png"), dest("test/res/test.png");
    Filter("Example").apply(&src, &dest, empty);
}