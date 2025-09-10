#pragma once
#include "base.hpp"
#include "exception.hpp"
/**
 * Rust-style option type
 */

LCORE_NAMESPACE_BEGIN

template <typename T, typename E>
class Result;

class None {};

template <typename T>
class Option {
    bool is_some;
    union {
        T value;
    };
public:
    Option(const T& v): is_some(true), value(v) {}
    Option(T&& v): is_some(true), value(std::move(v)) {}
    Option(const Option& o): is_some(o.is_some) {
        if (is_some) new(&value) T(o.value);
    }
    Option(Option&& o): is_some(o.is_some) {
        if (is_some) new(&value) T(std::move(o.value));
    }
    Option(const None&): is_some(false) {}
    Option(None&&): is_some(false) {}
    ~Option() {
        if (is_some) value.~T();
    }

    static Option Some(const T& v) { return Option(v); }
    static Option Some(T&& v) { return Option(std::move(v)); }
    static Option None() { return Option(None()); }
    
    bool IsSome() const { return is_some; }
    bool IsNone() const { return !is_some; }

    T& Unwrap() & {
        if (!is_some) throw RuntimeError("Called Unwrap on None");
        return value;
    }
    const T& Unwrap() const & {
        if (!is_some) throw RuntimeError("Called Unwrap on None");
        return value;
    }
    T&& Unwrap() && {
        if (!is_some) throw RuntimeError("Called Unwrap on None");
        return std::move(value);
    }

    T UnwrapOr(const T& default_value) const & {
        if (is_some) return value;
        return default_value;
    }
    T UnwrapOr(T&& default_value) && {
        if (is_some) return std::move(value);
        return std::move(default_value);
    }
    template <typename F>
    T UnwrapOrElse(F&& f) const & {
        if (is_some) return value;
        return f();
    }
    template <typename F>
    T UnwrapOrElse(F&& f) && {
        if (is_some) return std::move(value);
        return f();
    }
    template <typename U>
    Option<U> Map(std::function<U(const T&)> f) const & {
        if (is_some) return Option<U>(f(value));
        return Option<U>();
    }
    template <typename U>
    Option<U> Map(std::function<U(T&&)> f) && {
        if (is_some) return Option<U>(f(std::move(value)));
        return Option<U>();
    }
    template <typename U>
    Option<U> AndThen(std::function<Option<U>(const T&)> f) const & {
        if (is_some) return f(value);
        return Option<U>();
    }
    template <typename U>
    Option<U> AndThen(std::function<Option<U>(T&&)> f) && {
        if (is_some) return f(std::move(value));
        return Option<U>();
    }
    template <typename F>
    Option<T> OrElse(F&& f) const & {
        if (is_some) return *this;
        return f();
    }
    template <typename F>
    Option<T> OrElse(F&& f) && {
        if (is_some) return std::move(*this);
        return f();
    }
    template <typename F>
    Result<T, E> Transpose(F&& f) const & {
        if (is_some) {
            return f(value);
        } else {
            return Result<T, E>(None());
        }
    }
    template <typename F>
    Result<T, E> Transpose(F&& f) && {
        if (is_some) {
            return f(std::move(value));
        } else {
            return Result<T, E>(None());
        }
    }
};

template <typename T>
Option<T> Some(const T& v) { return Option<T>::Some(v); }
template <typename T>
Option<T> Some(T&& v) { return Option<T>::Some(std::move(v)); }

namespace detail {

/// @brief If a value have a null state (e.g. pointer, smart pointer), this option type can be used to represent an optional value without extra memory allocation.
template <typename T, auto NullValue = nullptr>
class NullStateOption {
    T value;
public:
    static constexpr T null_value = NullValue;

    NullStateOption(const T& v): value(v) {}
    NullStateOption(T&& v): value(std::move(v)) {}
    NullStateOption(const NullStateOption& o): value(o.value) {}
    NullStateOption(NullStateOption&& o): value(std::move(o.value)) {}
    NullStateOption(const None&): value(null_value) {}
    NullStateOption(None&&): value(null_value) {}
    ~NullStateOption() = default;

    static NullStateOption Some(const T& v) { return NullStateOption(v); }
    static NullStateOption Some(T&& v) { return NullStateOption(std::move(v)); }
    static NullStateOption None() { return NullStateOption(None()); }

    bool IsSome() const { return value != null_value; }
    bool IsNone() const { return value == null_value; }
    T& Unwrap() & {
        if (IsNone()) throw RuntimeError("Called Unwrap on None");
        return value;
    }
    const T& Unwrap() const & {
        if (IsNone()) throw RuntimeError("Called Unwrap on None");
        return value;
    }
    T&& Unwrap() && {
        if (IsNone()) throw RuntimeError("Called Unwrap on None");
        return std::move(value);
    }
    T UnwrapOr(const T& default_value) const & {
        if (IsSome()) return value;
        return default_value;
    }
    T UnwrapOr(T&& default_value) && {
        if (IsSome()) return std::move(value);
        return std::move(default_value);
    }
    template <typename F>
    T UnwrapOrElse(F&& f) const & {
        if (IsSome()) return value;
        return f();
    }
    template <typename F>
    T UnwrapOrElse(F&& f) && {
        if (IsSome()) return std::move(value);
        return f();
    }
    template <typename U>
    NullStateOption<U, NullValue> Map(std::function<U(const T&)> f) const & {
        if (IsSome()) return NullStateOption<U, NullValue>(f(value));
        return NullStateOption<U, NullValue>();
    }
    template <typename U>
    NullStateOption<U, NullValue> Map(std::function<U(T&&)> f) && {
        if (IsSome()) return NullStateOption<U, NullValue>(f(std::move(value)));
        return NullStateOption<U, NullValue>();
    }
    template <typename U>
    NullStateOption<U, NullValue> AndThen(std::function<NullStateOption<U, NullValue>(const T&)> f) const & {
        if (IsSome()) return f(value);
        return NullStateOption<U, NullValue>();
    }
    template <typename U>
    NullStateOption<U, NullValue> AndThen(std::function<NullStateOption<U, NullValue>(T&&)> f) && {
        if (IsSome()) return f(std::move(value));
        return NullStateOption<U, NullValue>();
    }
    template <typename F>
    NullStateOption<T, NullValue> OrElse(F&& f) const & {
        if (IsSome()) return *this;
        return f();
    }
    template <typename F>
    NullStateOption<T, NullValue> OrElse(F&& f) && {
        if (IsSome()) return std::move(*this);
        return f();
    }
    template <typename F, typename E>
    Result<T, E> Transpose(F&& f) const & {
        if (IsSome()) {
            return f(value);
        } else {
            return Result<T, E>(None());
        }
    }
    template <typename F, typename E>
    Result<T, E> Transpose(F&& f) && {
        if (IsSome()) {
            return f(std::move(value));
        } else {
            return Result<T, E>(None());
        }
    }
};

}

template <typename T>
class RawPtr;
template <typename T>
class SharedPtr;
template <typename T, typename Deleter>
class UniquePtr;

namespace detail {

template <typename T>
struct NullStateInfo;

template <typename T>
struct NullStateInfo<RawPtr<T>> {
    static constexpr auto null_value = nullptr;
};
template <typename T>
struct NullStateInfo<SharedPtr<T>> {
    static constexpr auto null_value = nullptr;
};
template <typename T, typename Deleter>
struct NullStateInfo<UniquePtr<T, Deleter>> {
    static constexpr auto null_value = nullptr;
};

template <typename T>
concept IsNullStateOptionCompatible = requires {
    { NullStateInfo<T>::null_value };
};

}

template <typename T>
requires detail::IsNullStateOptionCompatible<T>
class Option<T>: public detail::NullStateOption<T, detail::NullStateInfo<T>::null_value> {
    using Base = detail::NullStateOption<T, detail::NullStateInfo<T>::null_value>;
public:
    using Base::Base;
    inline static Option Some(const T& v) { return Base::Some(v); }
    inline static Option Some(T&& v) { return Base::Some(std::move(v)); }
    inline static Option None() { return Base::None(); }
};

LCORE_NAMESPACE_END

#ifdef __GNUC__
#define L_TRYO ({ auto _l_tryo_result = (expr); if (_l_tryo_result.IsNone()) return None(); _l_tryo_result.Unwrap(); })
#endif
