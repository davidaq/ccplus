#include "gtest/gtest.h" 

#include "global.hpp"

using namespace CCPlus;

int testValue = 0;

class TestA : public Object {
public:
    ~TestA() {
        testValue += 10;
    }
};

class TestB : public Object {
public:
    ~TestB() {
        testValue += 1;
    }
};

TEST(ObjectTest, RetainAndDelete) {
    if(true) {
        testValue = 0;
        TestA a;
        a.retain(new TestB());
    }
    EXPECT_EQ(testValue, 10 + 1);
}

TEST(ObjectTest, DoubleRetain) {
    if(true) {
        testValue = 0;
        TestA a1;
        TestA a2;
        TestB tb;
        TestB* b = new TestB();
        a1.retain(b);
        a2.retain(b);
    }
    EXPECT_EQ(testValue, 20 + 2);
}


TEST(ObjectTest, RetainTransport) {
    testValue = 0;
    TestA* a1 = new TestA();
    TestA* a2 = new TestA();
    TestB* b = new TestB();
    a1->retain(b);
    a2->retain(b);
    delete a1;
    EXPECT_EQ(testValue, 10);
    testValue = 0;
    delete a2;
    EXPECT_EQ(testValue, 11);
}

TEST(ObjectTest, ChainRetain) {
    if(true) {
        testValue = 0;
        TestA a;
        TestA* pa;
        TestB* pb;
        a.retain(pa = new TestA());
        pa->retain(pb = new TestB());
        pb->retain(pa = new TestA());
    }
    EXPECT_EQ(testValue, 30 + 1);
}
