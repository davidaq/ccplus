#include "gtest/gtest.h"

#include "ccplus.hpp"
#include "utils.hpp"

#include <boost/property_tree/json_parser.hpp> 
#include <boost/property_tree/ptree.hpp> 

#include "logger.hpp"

using namespace std;
using namespace CCPlus;

TEST(FillTML, FillTML) {
    string retpath = "tmp/test5.result.tml";
    CCPlus::fillTML("test/res/testfilltml.json", "test/res/test5.tml", retpath);
    using boost::property_tree::ptree;
    ptree t1, t2;
    read_json("tmp/test5.result.tml", t1);
    read_json("test/res/test5.result.tml", t2);

    EXPECT_EQ(10.0, t1.get<float>("compositions.MAIN.duration"));
    auto& l = (*t1.get_child("compositions.__0.layers").begin()).second;
    EXPECT_EQ("file://-109784687-126-86-1-4646-29115-2-101-63101-111.jpg", l.get<std::string>("uri"));

    auto& transP = l.get_child("properties.transform.0");
    auto ite = transP.begin();
    EXPECT_EQ(7, transP.size());
    std::advance(ite, 2);
    EXPECT_EQ(2, std::atof((*ite).second.data().c_str()));
    std::advance(ite, 1);
    EXPECT_EQ(1, std::atof((*ite).second.data().c_str()));
}
