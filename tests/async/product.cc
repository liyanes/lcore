#include <gtest/gtest.h>
#include <lcore/async/utils.hpp>
#include <lcore/traits.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <tuple>

using namespace LCORE_NAMESPACE_NAME::async;

TEST(ProductTest, BasicFunctionality) {
    std::vector<int> vec = {1, 2, 3};
    std::list<int> lst = {4, 5};
    std::vector<double> dvec = {1.1, 2.2};

    auto pd = product(vec, lst, dvec);
    std::vector<std::tuple<int, int, double>> results;

    for (auto [v, l, d] : pd) {
        results.emplace_back(v, l, d);
    }

    ASSERT_EQ(results.size(), 12);
    EXPECT_EQ(results[0], std::make_tuple(1, 4, 1.1));
    EXPECT_EQ(results[1], std::make_tuple(1, 4, 2.2));
    EXPECT_EQ(results[2], std::make_tuple(1, 5, 1.1));
    EXPECT_EQ(results[3], std::make_tuple(1, 5, 2.2));
    EXPECT_EQ(results[4], std::make_tuple(2, 4, 1.1));
    EXPECT_EQ(results[5], std::make_tuple(2, 4, 2.2));
}

TEST(ProductTest, EmptyContainer) {
    std::vector<int> vec = {};
    std::list<int> lst = {4, 5};

    auto pd = product(vec, lst);
    std::vector<std::tuple<int, int>> results;

    for (auto [v, l] : pd) {
        results.emplace_back(v, l);
    }

    ASSERT_TRUE(results.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
