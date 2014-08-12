#include "gtest/gtest.h"

#include "frame.hpp"
#include "logger.hpp"

#include <vector>
#include <map>

using namespace std;
using namespace CCPlus;

TEST(Experiment, MapExperiment1) {
    std::map<std::string, std::vector<int>> mp;
    mp["first"] = vector<int>({1, 2, 3, 4, 5});
    L() << &mp["first"].front();
    mp["second"] = vector<int>({5, 6, 7, 8});
    L() << "first: " << &mp["first"].front();
    L() << "second: " << &mp["second"].front();

    mp["1econd"] = vector<int>({5, 6, 7, 8});
    L() << "first: " << &mp["first"].front();
    L() << "second: " << &mp["second"].front();

    vector<int>& tmp = mp.at("first");
    tmp.push_back(1243);
    L() << "first: " << &mp["first"].front();
    L() << "second: " << &mp["second"].front();
}

TEST(Experiment, MapExperiment2) {
    std::map<std::string, std::vector<int>> mp;
    mp["first"] = vector<int>({1, 2, 3, 4, 5});
    L() << &mp["first"];

    mp["second"] = vector<int>({5, 6, 7, 8});
    L() << &mp["first"];

    mp["1econd"] = vector<int>({5, 6, 7, 8});
    L() << &mp["first"];
}

TEST(Experiment, VectorExperiment) {
    vector<int> vec = {5, 6, 7};
    L() << &vec.front();
    vec.push_back(9);
    L() << &vec.front();
}

TEST(Experiment, TEST) {
    //Frame f("test/res/6cd76271-8f09-42d8-a507-72409d3c4ac6_0.zim");
    //L() << (int)f.getAudio().total();
}
