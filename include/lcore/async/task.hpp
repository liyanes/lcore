#pragma once
#include "base.hpp"
#include "lcore/class.hpp"
#include "lcore/traits.hpp"
#include <coroutine>
#include <utility>
#include <optional>

LCORE_ASYNC_NAMESPACE_BEGIN

template <typename InitSuspend = std::suspend_never, typename FinalSuspend = std::suspend_always>
class SuspendHandler {
public:
    using InitSuspendType = InitSuspend;
    using FinalSuspendType = FinalSuspend;

    auto initial_suspend() noexcept { return InitSuspend();}
    auto final_suspend() noexcept { return FinalSuspend(); }
};

template <typename T, typename SuspendHandlerType = SuspendHandler<>>
class Task;

template <typename T>
using DefaultTaskWrapper = Task<T, std::suspend_never, std::suspend_always>;

template <typename T, template <typename> typename TaskType = DefaultTaskWrapper, typename SuspendHandleType = SuspendHandler<>>
class Promise: public SuspendHandleType {
public:
    using value_type = T;
    using promise_type = Promise<T, TaskType, SuspendHandleType>;
    using handle_type = std::coroutine_handle<promise_type>;

    Promise() = default;
    ~Promise() = default;

    auto get_return_object(){
        return TaskType<T>(handle_type::from_promise(*this));
    }

    void unhandled_exception(){
        throw;
    }

    void return_value(T&& value){
        this->value = std::move(value);
    }

    void return_value(const T& value){
        this->value = value;
    }

    std::optional<T> value;
};

template <template <typename> typename TaskType, typename SuspendHandleType>
class Promise<void, TaskType>: public SuspendHandleType {
public:
    using value_type = void;
    using promise_type = Promise<void, TaskType, SuspendHandleType>;
    using handle_type = std::coroutine_handle<promise_type>;

    Promise() = default;
    ~Promise() = default;

    auto get_return_object(){
        return TaskType<void>(handle_type::from_promise(*this));
    }

    void unhandled_exception(){
        throw;
    }

    void return_void(){
    }
};

/// @brief Awaitable base interface, used in co_await
/// @tparam T 
template <typename T>
class AwaitableBase: public Interface {
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

template <typename T, typename SuspendHandlerType = SuspendHandler<>>
class Task: AwaitableBase<T>{
public:
    using promise_type = Promise<T, Task, SuspendHandlerType>;
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

    Task& operator=(const Task&) = delete;
    Task& operator=(Task&& other){
        handle = other.handle;
        other.handle = nullptr;
        return *this;
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

template <typename SuspendHandlerType>
class Task<void, SuspendHandlerType>: AwaitableBase<void>{
public:
    using promise_type = Promise<void, Task, SuspendHandlerType>;
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

    Task& operator=(const Task&) = delete;
    Task& operator=(Task&& other){
        handle = other.handle;
        other.handle = nullptr;
        return *this;
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

LCORE_ASYNC_NAMESPACE_END

