#pragma once
#include "../config.h"
#include "../class.hpp"
#include <coroutine>
#include <utility>
#include <optional>

LCORE_NAMESPACE_BEGIN

template <typename T>
class Task;

template <typename T, template <typename> typename TaskType>
class Promise {
public:
    using value_type = T;
    using handle_type = std::coroutine_handle<Promise<T, TaskType>>;
    using promise_type = Promise<T, TaskType>;

    Promise() = default;
    ~Promise() = default;

    auto get_return_object(){
        return TaskType<T>(handle_type::from_promise(*this));
    }

    auto initial_suspend() noexcept {
        return std::suspend_never();        // Do not suspend the coroutine at the beginning
    }

    auto final_suspend() noexcept {
        return std::suspend_always();       // Suspend the coroutine at the end
    }

    void unhandled_exception(){
    }

    void return_value(T&& value){
        this->value = std::move(value);
    }

    void return_value(const T& value){
        this->value = value;
    }

    std::optional<T> value;
};

template <template <typename> typename TaskType>
class Promise<void, TaskType> {
public:
    using value_type = void;
    using handle_type = std::coroutine_handle<Promise<void, TaskType>>;
    using promise_type = Promise<void, TaskType>;

    Promise() = default;
    ~Promise() = default;

    auto get_return_object(){
        return TaskType<void>(handle_type::from_promise(*this));
    }

    auto initial_suspend() noexcept {
        return std::suspend_never();        // Do not suspend the coroutine at the beginning
    }

    auto final_suspend() noexcept {
        return std::suspend_always();       // Suspend the coroutine at the end
    }

    void unhandled_exception(){
    }

    void return_void(){
    }
};

/// @brief Awaitable base interface, used in co_await
/// @tparam T 
template <typename T>
class AwaitableBase: public Interface{
    bool await_ready() {return false;};
    bool await_suspend(std::coroutine_handle<> handle) {return false;};
    T await_resume() {return T{};};
};

/// @brief Awaitable abstract class, used in co_await and executor
/// @tparam T 
template <typename T>
class Awaitable: public AbstractClass {
public:
    virtual bool await_ready() = 0;
    virtual bool await_suspend(std::coroutine_handle<> handle) = 0;
    virtual T await_resume() = 0;
};

template <typename T>
class Task: AwaitableBase<T>{
public:
    using promise_type = Promise<T, Task>;
    struct sentinel{};
private:
    std::coroutine_handle<promise_type> handle;
public:
    Task(): handle(nullptr){
    }
    Task(std::coroutine_handle<promise_type> handle): handle(handle){
    }

    Task(const Task&) = delete;
    Task(Task&& other): handle(other.handle){
        other.handle = nullptr;
    }

    ~Task(){
        if(handle) handle.destroy();
    }

    bool await_ready(){
        return this->handle.promise().value.has_value() || this->handle.done();
    }

    void await_suspend(std::coroutine_handle<promise_type> handle){
        this->handle = handle;
    }

    T await_resume(){
        return this->handle.promise().value.value();
    }

    bool has_value(){
        return handle.promise().value.has_value();
    }

    T get(){
        return handle.promise().value.value();
    }

    void resume(){
        handle.resume();
    }

    bool done(){
        return handle.done();
    }
};

template <>
class Task<void>: AwaitableBase<void>{
public:
    using promise_type = Promise<void, Task>;
    struct sentinel{};
private:
    std::coroutine_handle<promise_type> handle;
public:
    Task(): handle(nullptr){
    }
    Task(std::coroutine_handle<promise_type> handle): handle(handle){
    }

    Task(const Task&) = delete;
    Task(Task&& other): handle(other.handle){
        other.handle = nullptr;
    }

    ~Task(){
        if(handle) handle.destroy();
    }

    bool await_ready(){
        return handle.done();
    }

    void await_suspend(std::coroutine_handle<promise_type> handle){
        this->handle = handle;
    }

    void await_resume(){
    }

    void resume(){
        handle.resume();
    }

    bool done(){
        return handle.done();
    }
};

LCORE_NAMESPACE_END

