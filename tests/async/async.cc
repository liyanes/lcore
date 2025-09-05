#include <gtest/gtest.h>
#include <lcore/async/task.hpp>
#include <lcore/async/generator.hpp>
#include <lcore/async/agenerator.hpp>
#include <iostream>
#include <ranges>
#include <memory>

using namespace LCORE_NAMESPACE_NAME::async;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

Generator<int> yiledtest(){
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

TEST(AsyncTest, GeneratorTest) {
    std::vector<int> results;
    for (auto i : yiledtest()) {
        results.push_back(i);
    }
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
    EXPECT_EQ(results[2], 3);
}

Generator<std::shared_ptr<int>> yiledtest2(){
    co_yield std::make_shared<int>(1);
    co_yield std::make_shared<int>(2);
    co_yield std::make_shared<int>(3);
}

TEST(AsyncTest, GeneratorTest2) {
    std::vector<std::shared_ptr<int>> results;
    for (auto i : yiledtest2()) {
        results.push_back(i);
    }
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(*results[0], 1);
    EXPECT_EQ(*results[1], 2);
    EXPECT_EQ(*results[2], 3);
}

Generator<int> viewtest(){
    int i = 0;
    while (true){
        co_yield i++;
    }
}

TEST(AsyncTest, ViewTest) {
    std::vector<int> results;
    for (auto i : viewtest() | std::views::take(10)) {
        results.push_back(i);
    }
    ASSERT_EQ(results.size(), 10);
    for (int j = 0; j < 10; ++j) {
        EXPECT_EQ(results[j], j);
    }
}

Task<int> tasktest(){
    co_return 1;
};

TEST(AsyncTest, TaskTest) {
    auto task = tasktest();
    while (!task.done()) task.resume();
    ASSERT_TRUE(task.done());
    EXPECT_EQ(task.ref_value(), 1);
}

Task<int> taskawaittest(){
    co_await tasktest();
    co_return 2;
};

TEST(AsyncTest, TaskAwaitTest) {
    auto task = taskawaittest();
    while (!task.done()) task.resume();
    ASSERT_TRUE(task.done());
    EXPECT_EQ(task.ref_value(), 2);
}

AsyncGenerator<int> agen(){
    co_await std::suspend_always();
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

TEST(AsyncTest, AsyncGeneratorTest) {
    std::vector<int> results;
    for (auto i : agen()) {
        while (!i.done()) i.resume();
        auto value = i.ref_value();
        // In async generator, we need to manually check if the generator will yield a value
        if (!value.has_value()) break; // Check if the generator will yield a value, if not, break the loop
        results.push_back(value.value());
    }
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
    EXPECT_EQ(results[2], 3);
}

Task<void> voidtext(){
    co_await std::suspend_always();
    // std::cout << "Void test" << std::endl;
}

TEST(AsyncTest, VoidTest) {
    auto task = voidtext();
    while (!task.done()) task.resume();
    // No assertion here, just checking if it runs without crashing
}

TEST(AsyncTest, MoveOnlyTypeTest) {
    auto task = []() -> Task<std::unique_ptr<int>> {
        co_await std::suspend_always();
        co_return std::make_unique<int>(42);
    }();
    while (!task.done()) task.resume();
    ASSERT_TRUE(task.done());
    auto& value = task.ref_value();
    ASSERT_TRUE(value != nullptr);
    EXPECT_EQ(*value, 42);
}
