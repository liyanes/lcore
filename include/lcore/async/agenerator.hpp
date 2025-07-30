#include "base.hpp"
#include "task.hpp"
#include "traits.hpp"
#include <coroutine>
#include <optional>

LCORE_ASYNC_NAMESPACE_BEGIN

// Due to the lack of support for async generator in C++20/23, we need to implement it ourselves
// the returns of begin() and end() will be awaitable objects, and the generator will be a coroutine

template <typename T>
class AsyncGenerator;

template <typename T>
class __agen_promise {
public:
    using value_type = T;
    using handle_type = std::coroutine_handle<__agen_promise<T>>;
    using promise_type = __agen_promise<T>;

    __agen_promise() = default;
    ~__agen_promise() = default;

    auto get_return_object(){
        return AsyncGenerator<T>(handle_type::from_promise(*this));
    }

    auto initial_suspend() noexcept {
        return std::suspend_always();        // Do not suspend the coroutine at the beginning
    }

    auto final_suspend() noexcept {
        return std::suspend_always();       // Suspend the coroutine at the end
    }

    void unhandled_exception(){
        throw;
    }

    auto yield_value(T&& value){
        this->value = std::move(value);
        return std::suspend_always();
    }

    std::optional<T> value;
};

/**
 * @brief Async generator, used to generate a sequence of values asynchronously
 * For example:
 * @code{.cpp}
 * AsyncGenerator<int> agen(){
 *    co_await std::suspend_always();
 *    co_yield 1;
 *    co_yield 2;
 *    co_yield 3;
 *    co_await std::suspend_always();
 * }
 * @endcode
 * When using the generator, we need to manually check if the generator will yield a value
 * @code{.cpp}
 * for (auto i: agen()){
 *    while (!i.done()) i.resume();
 *    if (!i.has_value()) break;            // Check if the generator will yield a value (after the generator yield the last value, it will execute the unfinished code after co_yield)
 *    std::cout << i.get() << std::endl;
 * }
 * @endcode
 * @tparam T 
 */
template <typename T>
class AsyncGenerator {
public:
    using promise_type = __agen_promise<T>;
    using handle_type = std::coroutine_handle<promise_type>;
    struct sentinel {};
private:
    handle_type handle;
public:

    explicit AsyncGenerator(handle_type handle) : handle(handle) {}

    AsyncGenerator(AsyncGenerator const&) = delete;
    AsyncGenerator(AsyncGenerator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    ~AsyncGenerator() {
        if(handle) handle.destroy();
    }
    AsyncGenerator& operator=(AsyncGenerator const&) = delete;
    AsyncGenerator& operator=(AsyncGenerator&& other) noexcept {
        if(handle) handle.destroy();
        handle = other.handle;
        other.handle = nullptr;
        return *this;
    }

    class iterator {
        handle_type handle;
        int next = 1;
    public:
        iterator() = default;
        explicit iterator(handle_type handle) : handle(handle) {}

        iterator& operator++() {
            next++;
            return *this;
        }

        void operator++(int) {
            operator++();
        }

        bool operator!=(iterator const& other) const noexcept {
            return handle != other.handle;
        }
        
        bool operator!=(sentinel const&) const noexcept {
            return !handle.done();
        }

        Task<std::optional<T>> operator*() {
            auto &promise = handle.promise();
            while (next--) {
                promise.value = std::nullopt;
                while (!handle.done() && !promise.value.has_value()) {
                    handle.resume();
                    co_await std::suspend_always();
                }
                if (handle.done()) {
                    if (promise.value.has_value()) {
                        co_return promise.value;
                    }
                    co_return std::nullopt; // No more values to yield
                }
            }
            next = 0;
            co_return promise.value.value();
        }
    };

    iterator begin() {
        return iterator{handle};
    }

    sentinel end() {
        return {};
    }
};

LCORE_ASYNC_NAMESPACE_END
