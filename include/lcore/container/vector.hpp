#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <vector>
#include <algorithm>

LCORE_NAMESPACE_BEGIN

template <typename T, typename Allocator = std::allocator<T>>
class Vector: public std::vector<T, Allocator> {
public:
    using std::vector<T, Allocator>::vector;

    template <Iterable tContainer>
    requires IsSame<T, IterableValueType<tContainer>>
    inline Vector(const tContainer& container): std::vector<T, Allocator>::vector(container.begin(), container.end()) {};

    inline bool contains(const T& value) const {
        return std::find(std::begin(*this), std::end(*this), value) != std::end(*this);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && IsSame<ResultCallable<Func, const T&>, bool>
    inline bool contains(Func f) const {
        return std::find_if(std::begin(*this), std::end(*this), f) != std::end(*this);
    }

    inline void insert_unique(const T& value) {
        if (!contains(value)) {
            this->push_back(value);
        }
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && IsSame<ResultCallable<Func, const T&>, bool>
    inline void remove_if(Func f){
        std::erase(std::remove_if(std::begin(*this), std::end(*this), f), std::end(*this));
    }

    template <Iterable tContainer>
    requires IsSame<IterableValueType<tContainer>, T>
    inline void extends(const tContainer& other){
        this->insert(std::end(*this), std::begin(other), std::end(other));
    }

    template <Iterable tContainer, typename Func>
    requires IsSame<T, ResultCallable<Func, const IterableValueType<tContainer>&>>
    inline static Vector<T> fromContainer(const tContainer& iterable, Func f) {
        Vector<T> res;
        if constexpr (Integer<decltype(std::declval<tContainer>().size())>) {
            res.reserve(iterable.size());
        }
        for (auto& item: iterable) {
            res.push_back(f(item));
        }
        return res;
    }
};

LCORE_NAMESPACE_END
