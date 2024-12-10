#pragma once
#include "base.hpp"

#include <map>
#include <functional>
#include <initializer_list>

LCORE_NAMESPACE_BEGIN

/// @brief Map is a simple key-value map
/// @tparam K Key type
/// @tparam V Value type
/// @tparam Compare Compare function
/// @tparam Allocator Alloctor type
template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K,V>>>
using Map = std::map<K, V, Compare, Allocator>;

/// @brief BiMap is a bidirectional map
/// @tparam L Left type
/// @tparam R Right type
/// @tparam CompareL Left compare function
/// @tparam CompareR Right compare function
/// @tparam allocator Allocator type
template <typename L, typename R, typename CompareL = std::less<L>, typename CompareR = std::less<R>, 
        template <typename T> typename allocator = std::allocator>
class Bimap {
public:
    using LAllocator = allocator<std::pair<const L, R>>;
    using RAllocator = allocator<std::pair<const R, L>>;

    using LeftMap = Map<L, R, CompareL, LAllocator>;
    using RightMap = Map<R, L, CompareR, RAllocator>;
private:
    LeftMap left;
    RightMap right;
public:
    void insert(const L& l, const R& r){
        left.insert({l, r});
        right.insert({r, l});
    }

    const R& getRight(const L& l) const {
        return left.at(l);
    }

    const L& getLeft(const R& r) const {
        return right.at(r);
    }

    const LeftMap& getLeftMap() const {
        return left;
    }

    const RightMap& getRightMap() const {
        return right;
    }

    void clear(){
        left.clear();
        right.clear();
    }

    Bimap() = default;
    Bimap(const Bimap&) = default;
    Bimap(Bimap&&) = default;

    Bimap& operator=(const Bimap&) = default;

    Bimap(std::initializer_list<std::pair<L, R>> list){
        for (auto& [l, r]: list){
            insert(l, r);
        }
    }

    Bimap(const std::map<L, R>& lmap){
        for (auto& [l, r]: lmap){
            insert(l, r);
        }
    }
};

LCORE_NAMESPACE_END
