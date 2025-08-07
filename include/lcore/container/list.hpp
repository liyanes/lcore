#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <list>

LCORE_NAMESPACE_BEGIN

template <typename T, typename Allocator = std::allocator<T>>
class List: public std::list<T, Allocator> {
public:
    using std::list<T, Allocator>::list;
    using iterator = typename std::list<T, Allocator>::iterator;
    using const_iterator = typename std::list<T, Allocator>::const_iterator;
    using reserve_iterator = typename std::list<T, Allocator>::reverse_iterator;
    using const_reserve_iterator = typename std::list<T, Allocator>::const_reverse_iterator;

    using value_type = T;
    using reference = typename std::list<T, Allocator>::reference;
    using const_reference = typename std::list<T, Allocator>::const_reference;

    using size_type = typename std::list<T, Allocator>::size_type;
    using difference_type = typename std::list<T, Allocator>::difference_type;

    using allocator_type = Allocator;

    inline bool contains(const T& value) const {
        return std::find(std::begin(*this), std::end(*this), value) != std::end(*this);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
    inline bool contains(Func f) const {
        return std::find_if(std::begin(*this), std::end(*this), f) != std::end(*this);
    }

    inline void insert_unique(const T& value) {
        if (!contains(value)) {
            this->push_back(value);
        }
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
    inline void remove_if(Func f){
        std::erase(std::remove_if(std::begin(*this), std::end(*this), f), std::end(*this));
    }

    template <Iterable tContainer>
    requires Same<IterableValueType<tContainer>, T>
    inline void extends(const tContainer& other){
        this->insert(std::end(*this), std::begin(other), std::end(other));
    }

    inline iterator find(const T& value) {
        return std::find(std::begin(*this), std::end(*this), value);
    }
    inline const_iterator find(const T& value) const {
        return std::find(std::cbegin(this), std::cend(this), value);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
    inline iterator find_if(Func f) {
        return std::find_if(std::begin(*this), std::end(*this), f);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
    inline const_iterator find_if(Func f) const {
        return std::find_if(std::cbegin(this), std::cend(this), f);
    }
};

LCORE_NAMESPACE_END
