#include "ThreadPool.h"
#include "gtest/gtest.h"

using namespace ThreadDemo;


int f(const int& x) {
    return x * x;
}

TEST(TestThreadPool, TestSubmit) {
    ThreadPool pool{};
    int i = 10;
    auto fut = pool.submit(f, std::cref(i));
    ASSERT_EQ(f(i), fut.get());
}