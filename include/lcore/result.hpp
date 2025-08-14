#pragma once
#include "base.hpp"
#include "exception.hpp"
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

template <typename T>
class Error {
public:
    using ValueType = T;
    
private:
    T m_value;

public:
    constexpr Error() = default;
    constexpr Error(T&& value): m_value(std::move(value)) {}
    constexpr Error(const T& value): m_value(value) {}

    constexpr const T& Value() const { return m_value; }
    constexpr T& Value() { return m_value; }

    constexpr explicit operator bool() const { return true; }
    constexpr const T* operator->() const { return &m_value; }
    constexpr T* operator->() { return &m_value; }
    constexpr const T& operator*() const { return m_value; }
    constexpr T& operator*() { return m_value; }
};

template <typename T, typename E>
class Result {
public:
    using ValueType = T;
    using ErrorType = LCORE_NAMESPACE_NAME::Error<E>;
    using ErrorValueType = E;
    
private:
    union {
        ValueType value;
        ErrorType error;
    };
    bool isOk;

public:
    constexpr Result(): value(), isOk(true) {}
    constexpr Result(ValueType&& v): value(std::move(v)), isOk(true) {}
    constexpr Result(const ValueType& v): value(v), isOk(true) {}
    constexpr Result(ErrorType&& e): error(std::move(e)), isOk(false) {}
    constexpr Result(const ErrorType& e): error(e), isOk(false) {}

    constexpr ~Result() {
        if (isOk) value.~ValueType();
        else error.~ErrorType();
    }
    constexpr Result(const Result& other) : isOk(other.isOk) {
        if (isOk) new (&value) ValueType(other.value);
        else new (&error) ErrorType(other.error);
    }
    constexpr Result(Result&& other) noexcept : isOk(other.isOk) {
        if (isOk) new (&value) ValueType(std::move(other.value));
        else new (&error) ErrorType(std::move(other.error));
        other.isOk = true; // Reset the moved-from state
    }

    constexpr Result& operator=(const Result& other) {
        if (this != &other) {
            this->~Result();
            isOk = other.isOk;
            if (isOk) new (&value) ValueType(other.value);
            else new (&error) ErrorType(other.error);
        }
        return *this;
    }

    constexpr Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            this->~Result();
            isOk = other.isOk;
            if (isOk) new (&value) ValueType(std::move(other.value));
            else new (&error) ErrorType(std::move(other.error));
            other.isOk = true; // Reset the moved-from state
        }
        return *this;
    }

    constexpr bool IsOk() const { return isOk; }
    constexpr bool IsError() const { return !isOk; }
    constexpr T& Value() {
        if (!isOk) throw RuntimeError("Attempted to access value of an error Result");
        return value;
    }
    constexpr const T& Value() const {
        if (!isOk) throw RuntimeError("Attempted to access value of an error Result");
        return value;
    }
    constexpr E& Error() {
        if (isOk) throw RuntimeError("Attempted to access error of a successful Result");
        return error.Value();
    }
    constexpr const E& Error() const {
        if (isOk) throw RuntimeError("Attempted to access error of a successful Result");
        return error.Value();
    }

    constexpr explicit operator bool() const { return isOk; }
    constexpr T* operator->() {
        if (!isOk) throw RuntimeError("Attempted to access value of an error Result");
        return &value;
    }
    constexpr const T* operator->() const {
        if (!isOk) throw RuntimeError("Attempted to access value of an error Result");
        return &value;
    }
    constexpr T& operator*() {
        if (!isOk) throw RuntimeError("Attempted to dereference an error Result");
        return *value;
    }
    constexpr const T& operator*() const {
        if (!isOk) throw RuntimeError("Attempted to dereference an error Result");
        return *value;
    }

    T ValueOr(const T& defaultValue) const {
        return isOk ? value : defaultValue;
    }

    ErrorValueType ErrorOr(const ErrorValueType& defaultError) const {
        return isOk ? defaultError : *error;
    }

    template <typename Func>
    requires InvokeAble<Func, ValueType>
    auto Transform(Func&& func) const {
        if (!isOk) throw RuntimeError("Attempted to transform an error Result");
        using ReturnType = std::invoke_result_t<Func, ValueType>;
        return Result<ReturnType, ErrorValueType>(func(value));
    }

    template <typename Func>
    requires InvokeAble<Func, ErrorValueType>
    auto TransformError(Func&& func) const {
        if (isOk) throw RuntimeError("Attempted to transform a successful Result");
        using ReturnType = std::invoke_result_t<Func, ErrorValueType>;
        return Result<ValueType, ReturnType>(func(error.Value()));
    }

    template <typename Func>
    requires InvokeAble<Func, ValueType>
    auto AndThen(Func&& func) const {
        using ReturnType = std::invoke_result_t<Func, ValueType>;
        using ResultType = Result<ReturnType, ErrorValueType>;
        if (isOk) return ResultType(func(value));
        else return ResultType(error);
    }

    template <typename Func>
    requires InvokeAble<Func, ErrorValueType>
    auto OrElse(Func&& func) const {
        using ReturnType = std::invoke_result_t<Func, ErrorValueType>;
        using ResultType = Result<ValueType, ReturnType>;
        if (isOk) return ResultType(value);
        else return ResultType(func(error.Value()));
    }
};

template <typename E>
class Result<void, E> {
    using ValueType = void;
    using ErrorType = LCORE_NAMESPACE_NAME::Error<E>;
    using ErrorValueType = E;
    
private:
    union {
        char dummy; // Placeholder for void
        ErrorType error;
    };
    bool isOk;
    
public:
    constexpr Result(): dummy(0), isOk(true) {}
    constexpr Result(ErrorType&& e): error(std::move(e)), isOk(false) {}
    constexpr Result(const ErrorType& e): error(e), isOk(false) {}

    constexpr ~Result() {
        if (!isOk) error.~ErrorType();
    }
    constexpr Result(const Result& other) : isOk(other.isOk) {
        if (!isOk) new (&error) ErrorType(other.error);
    }
    constexpr Result(Result&& other) noexcept : isOk(other.isOk) {
        if (!isOk) new (&error) ErrorType(std::move(other.error));
        other.isOk = true; // Reset the moved-from state
    }

    constexpr Result& operator=(const Result& other) {
        if (this != &other) {
            this->~Result();
            isOk = other.isOk;
            if (!isOk) new (&error) ErrorType(other.error);
        }
        return *this;
    }

    constexpr Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            this->~Result();
            isOk = other.isOk;
            if (!isOk) new (&error) ErrorType(std::move(other.error));
            other.isOk = true; // Reset the moved-from state
        }
        return *this;
    }

    constexpr bool IsOk() const { return isOk; }
    constexpr bool IsError() const { return !isOk; }
    constexpr void Value() const {
        if (!isOk) throw RuntimeError("Attempted to access value of an error Result");
    }
    constexpr E& Error() {
        if (isOk) throw RuntimeError("Attempted to access error of a successful Result");
        return error.Value();
    }
    constexpr const E& Error() const {
        if (isOk) throw RuntimeError("Attempted to access error of a successful Result");
        return error.Value();
    }

    constexpr explicit operator bool() const { return isOk; }

    ErrorValueType ErrorOr(const ErrorValueType& defaultError) const {
        return isOk ? defaultError : *error;
    }

    template <typename Func>
    requires InvokeAble<Func>
    auto Transform(Func&& func) const {
        if (!isOk) throw RuntimeError("Attempted to transform an error Result");
        using ReturnType = std::invoke_result_t<Func>;
        return Result<ReturnType, ErrorValueType>(func());
    }

    template <typename Func>
    requires InvokeAble<Func, ErrorValueType>
    auto TransformError(Func&& func) const {
        if (isOk) throw RuntimeError("Attempted to transform a successful Result");
        using ReturnType = std::invoke_result_t<Func, ErrorValueType>;
        return Result<ValueType, ReturnType>(func(error.Value()));
    }

    template <typename Func>
    requires InvokeAble<Func>
    auto AndThen(Func&& func) const {
        using ReturnType = std::invoke_result_t<Func>;
        using ResultType = Result<ReturnType, ErrorValueType>;
        if (isOk) return ResultType(func());
        else return ResultType(error);
    }

    template <typename Func>
    requires InvokeAble<Func, ErrorValueType>
    auto OrElse(Func&& func) const {
        using ReturnType = std::invoke_result_t<Func, ErrorValueType>;
        using ResultType = Result<ValueType, ReturnType>;
        if (isOk) return ResultType();
        else return ResultType(func(error.Value()));
    }
};

LCORE_NAMESPACE_END
