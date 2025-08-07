#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <vector>
#include <algorithm>

LCORE_NAMESPACE_BEGIN


/**
 * @brief Container View
 * Use this class to create a view of a container,
 * this view can be used to slice, split, and iterate over the container
 * @tparam Container Container type
 * @tparam T Value type
 */
template <template <typename> typename Container, typename T>
requires Iterable<Container<T>>
class ContainerView {
public:
    using value_type = T;
    using iterator = typename Container<T>::iterator;
    using const_iterator = typename Container<T>::const_iterator;
    using reverse_iterator = typename Container<T>::reverse_iterator;
    using const_reverse_iterator = typename Container<T>::const_reverse_iterator;

    using view_type = ContainerView<Container, T>;
private:
    iterator begin_;
    iterator end_;
public:
    inline ContainerView(Container<T>& container): begin_(container.begin()), end_(container.end()){};
    inline ContainerView(iterator begin, iterator end): begin_(begin), end_(end){};

    inline iterator begin() { return begin_; }
    inline iterator end() { return end_; }
    inline const_iterator begin() const { return begin_; }
    inline const_iterator end() const { return end_; }
    inline const_iterator cbegin() const { return begin_; }
    inline const_iterator cend() const { return end_; }
    inline reverse_iterator rbegin() { return reverse_iterator(end_); }
    inline reverse_iterator rend() { return reverse_iterator(begin_); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin_); }
    inline const_reverse_iterator crbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator crend() const { return const_reverse_iterator(begin_); }

    inline size_t size() const { return std::distance(begin_, end_); }
    inline bool empty() const { return begin_ == end_; }

    inline T& front() { return *begin_; }
    inline T& back() { return *std::prev(end_); }

    inline const T& front() const { return *begin_; }
    inline const T& back() const { return *std::prev(end_); }

    inline void pop_front() { begin_++; }
    inline void pop_back() { end_--; }

    inline operator Container<T>() const{
        return Container<T>(begin_, end_);
    }

    inline T& operator[](size_t index){
        return *std::next(begin_, index);
    }

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
    requires IsCallable<Func, T&> && Same<ResultCallable<Func, T&>, bool>
    inline std::vector<view_type> split(Func f) {
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
    inline const_reverse_iterator crbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator crend() const { return const_reverse_iterator(begin_); }

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
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
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
class PointerView {
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;

    using view_type = PointerView<T>;
private:
    iterator begin_;
    iterator end_;
public:
    inline PointerView(T* begin, T* end): begin_(begin), end_(end){};

    inline iterator begin() { return begin_; }
    inline iterator end() { return end_; }
    inline const_iterator begin() const { return begin_; }
    inline const_iterator end() const { return end_; }
    inline const_iterator cbegin() const { return begin_; }
    inline const_iterator cend() const { return end_; }
    inline reverse_iterator rbegin() { return reverse_iterator(end_); }
    inline reverse_iterator rend() { return reverse_iterator(begin_); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin_); }
    inline const_reverse_iterator crbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator crend() const { return const_reverse_iterator(begin_); }

    inline size_t size() const { return std::distance(begin_, end_); }
    inline bool empty() const { return begin_ == end_; }

    inline T& front() { return *begin_; }
    inline T& back() { return *std::prev(end_); }

    inline const T& front() const { return *begin_; }
    inline const T& back() const { return *std::prev(end_); }

    inline void pop_front() { begin_++; }
    inline void pop_back() { end_--; }

    inline operator std::vector<T>() const{
        return std::vector<T>(begin_, end_);
    }

    inline T& operator[](size_t index){
        return *std::next(begin_, index);
    }

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
    requires IsCallable<Func, T&> && Same<ResultCallable<Func, T&>, bool>
    inline std::vector<view_type> split(Func f) {
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
class PointerView<const T> {
public:
    using value_type = const T;
    using iterator = const T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<const T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;

    using view_type = PointerView<const T>;
private:
    iterator begin_;
    iterator end_;
public:
    inline PointerView(const T* begin, const T* end): begin_(begin), end_(end){};

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
    inline const_reverse_iterator crbegin() const { return const_reverse_iterator(end_); }
    inline const_reverse_iterator crend() const { return const_reverse_iterator(begin_); }

    inline size_t size() const { return std::distance(begin_, end_); }
    inline bool empty() const { return begin_ == end_; }

    // inline T& front() { return *begin_; }
    // inline T& back() { return *std::prev(end_); }

    inline const T& front() const { return *begin_; }
    inline const T& back() const { return *std::prev(end_); }

    inline void pop_front() { begin_++; }
    inline void pop_back() { end_--; }

    inline operator std::vector<T>() const{
        return std::vector<T>(begin_, end_);
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
    requires IsCallable<Func, const T&> && Same<ResultCallable<Func, const T&>, bool>
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

LCORE_NAMESPACE_END
