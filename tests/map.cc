#include <gtest/gtest.h>
#include <lcore/map.hpp>
#include <lcore/pointer.hpp>

using namespace lcore;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(MapTest, WeakMap) {
    WeakMap<int, int> weakMap;
    auto ref1 = New<int>(100);
    auto ref2 = New<int>(200);
    weakMap.insert({1, ref1});
    weakMap.insert({2, ref2});

    weakMap.insert({3, WeakPtr<int>()});    // Invalid weak pointer

    EXPECT_EQ(weakMap.size(), 2);
    EXPECT_EQ(weakMap.count(1), 1);
    EXPECT_EQ(weakMap.count(2), 1);
    EXPECT_EQ(weakMap.count(3), 0); // Should return 0 for expired weak pointer

    auto it = weakMap.find(1);
    EXPECT_TRUE(it != weakMap.end());
    EXPECT_EQ(*it->second.Lock(), 100);

    it = weakMap.find(3);
    EXPECT_TRUE(it == weakMap.end()); // Should not find expired weak pointer

    weakMap.erase(2);
    EXPECT_EQ(weakMap.size(), 1);
    EXPECT_EQ(weakMap.count(2), 0);

    int looptime = 0;
    for (auto& [key, weakPtr] : weakMap) {
        if (looptime++ == 0) {
            EXPECT_EQ(key, 1);
            EXPECT_EQ(*weakPtr.Lock(), 100);
        } else {
            FAIL() << "Unexpected iteration over weakMap";
        }
    }
    
    // Weak reference loss
    ref1.Reset(); // Reset the shared pointer to make the weak pointer expire
    EXPECT_EQ(weakMap.count(1), 0); // Should return 0 for expired weak pointer

    // Check iterators
    auto itBegin = weakMap.begin();
    EXPECT_TRUE(itBegin == weakMap.end()); // Should be empty after expiration
}
