#pragma once
#include "base.hpp"
#include <functional>

LCORE_NAMESPACE_BEGIN

class ScopeGuard {
    bool active = true;
    std::function<void()> func;
public:
    explicit ScopeGuard(std::function<void()> func): func(std::move(func)) {}
    ~ScopeGuard() {
        if (active) func();
    }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept: active(other.active), func(std::move(other.func)) {
        other.Dismiss();
    }
    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            if (active) func();
            active = other.active;
            func = std::move(other.func);
            other.Dismiss();
        }
        return *this;
    }

    void Dismiss() noexcept { active = false; }
};

LCORE_NAMESPACE_END
