#pragma once
#include "base.hpp"
#include "traits.hpp"
#include <atomic>

LCORE_NAMESPACE_BEGIN

template <typename T>
class Synchronized {
    std::mutex mutex_;
    T value_;
public:
    Synchronized() = default;
    explicit Synchronized(const T& value) : value_(value) {}
    explicit Synchronized(T&& value) : value_(std::move(value)) {}
    Synchronized(const Synchronized&) = delete;
    Synchronized(Synchronized&&) = delete;
    Synchronized& operator=(const Synchronized&) = delete;
    Synchronized& operator=(Synchronized&&) = delete;

    template <typename Func>
    auto with_lock(Func&& func) -> decltype(func(std::declval<T&>())) {
        std::lock_guard<std::mutex> lock(mutex_);
        return func(value_);
    }

    template <typename Func>
    auto with_lock(Func&& func) const -> decltype(func(std::declval<const T&>())) {
        std::lock_guard<std::mutex> lock(mutex_);
        return func(value_);
    }
};

LCORE_NAMESPACE_END
