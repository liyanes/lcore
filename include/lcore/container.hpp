#pragma once
#include "base.hpp"
#include <list>
#include <vector>
#include <functional>
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

/**
 * @brief Const Container View
 * This view cannot modify the container
 * @tparam Container Container type
 * @tparam T Value type
 */
template <template <typename> typename Container, typename T>
requires Iterable<Container<T>>
class ConstContainerView {
public:
    using value_type = T;
    using iterator = typename Container<T>::const_iterator;
    using const_iterator = typename Container<T>::const_iterator;
    using reverse_iterator = typename Container<T>::const_reverse_iterator;
    using const_reverse_iterator = typename Container<T>::const_reverse_iterator;

    using view_type = ConstContainerView<Container, T>;
private:
    const_iterator begin_;
    const_iterator end_;

public:
    inline ConstContainerView(const Container<T>& container): begin_(container.begin()), end_(container.end()){};
    inline ConstContainerView(const_iterator begin, const_iterator end): begin_(begin), end_(end){};

    // inline iterator begin() { return begin_; }
    // inline iterator end() { return end_; }
    inline const_iterator begin() const { return begin_; }
    inline const_iterator end() const { return end_; }
    inline const_iterator cbegin() const { return begin_; }
    inline const_iterator cend() const { return end_; }
    // inline reverse_iterator rbegin() { return reverse_iterator(end_); }
    // inline reverse_iterator rend() { return reverse_iterator(begin_); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin_); }

    inline size_t size() const { return std::distance(begin_, end_); }
    inline bool empty() const { return begin_ == end_; }

    // inline T& front() { return *begin_; }
    // inline T& back() { return *std::prev(end_); }

    inline const T& front() const { return *begin_; }
    inline const T& back() const { return *std::prev(end_); }

    inline void pop_front() { begin_++; }
    inline void pop_back() { end_--; }

    inline operator Container<T>() const{
        return Container<T>(begin_, end_);
    }

    // inline T& operator[](size_t index){
    //     return *std::next(begin_, index);
    // }

    inline const T& operator[](size_t index) const {
        return *std::next(begin_, index);
    }

    inline view_type slice(size_t start, size_t end) const {
        return view_type(std::next(begin_, start), std::prev(begin_, end));
    }

    inline view_type slice(size_t start) const{
        return view_type(std::next(begin_, start), end_);
    }

    inline view_type subview(size_t start, size_t n = (size_t)-1) const {
        if (n == (size_t)-1) return view_type(std::next(begin_, start), end_);
        return view_type(std::next(begin_, start), std::next(begin_, start + n));
    }
    
    template <typename Func>
    requires IsCallable<Func, const T&> && IsSame<ResultCallable<Func, const T&>, bool>
    inline std::vector<view_type> split(Func f) const {
        std::vector<view_type> result;
        auto it = begin_;
        while(it != end_){
            auto next = std::find_if(it, end_, f);
            if(next != end_){
                result.push_back(view_type(it, next));
                it = std::next(next);
            } else {
                result.push_back(view_type(it, end_));
                break;
            }
        }
        return result;
    }
};

template <typename T>
using VectorView = ConstContainerView<std::vector, T>;

template <typename T>
using ListView = ConstContainerView<std::list, T>;

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

template <typename T, typename Allocator = std::allocator<T>>
class List: public std::list<T, Allocator> {
public:
    using std::list<T, Allocator>::list;
    using iterator = typename std::list<T, Allocator>::iterator;
    using const_iterator = typename std::list<T, Allocator>::const_iterator;

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

    inline iterator find(const T& value) {
        return std::find(std::begin(*this), std::end(*this), value);
    }
    inline const_iterator find(const T& value) const {
        return std::find(std::cbegin(this), std::cend(this), value);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && IsSame<ResultCallable<Func, const T&>, bool>
    inline iterator find_if(Func f) {
        return std::find_if(std::begin(*this), std::end(*this), f);
    }

    template <typename Func>
    requires IsCallable<Func, const T&> && IsSame<ResultCallable<Func, const T&>, bool>
    inline const_iterator find_if(Func f) const {
        return std::find_if(std::cbegin(this), std::cend(this), f);
    }
};

LCORE_NAMESPACE_END

