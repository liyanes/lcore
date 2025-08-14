#pragma once
#include "base.hpp"
#include "lcore/traits.hpp"
#include <optional>

LCORE_ASYNC_NAMESPACE_BEGIN

namespace detail {

template <typename Starter, typename... Args>
struct CallbackAwaiter {
    using tuple_t = std::tuple<std::decay_t<Args>...>;

    Starter starter;
    alignas(tuple_t) unsigned char storage[sizeof(tuple_t)];
    bool has_value = false;

    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        starter([this, h](Args&&... values) {
            ::new (storage) tuple_t(std::forward<Args>(values)...);
            has_value = true;
            h.resume();
        });
    }

    auto await_resume() {
        auto* tup = reinterpret_cast<tuple_t*>(storage);
        if constexpr (sizeof...(Args) == 1) {
            auto&& val = std::get<0>(*tup);
            using RetT = std::tuple_element_t<0, tuple_t>;
            RetT ret = std::move(val);
            tup->~tuple_t();
            return ret;
        } else {
            tuple_t ret = std::move(*tup);
            tup->~tuple_t();
            return ret;
        }
    }
};

template <typename Starter, typename ArgTuple>
struct _CallbackAwaiterHelper;

template <typename Starter, typename... Args>
struct _CallbackAwaiterHelper<Starter, std::tuple<Args...>> {
    using type = CallbackAwaiter<Starter, Args...>;
};

template <typename Starter, typename ArgTuple>
using _CallbackAwaiterHelper_t = typename _CallbackAwaiterHelper<Starter, ArgTuple>::type;

}

template <typename Starter>
auto MakeCallbackAwaiter(Starter&& starter) {
    using Callback = NthTypeOfTuple<0, typename FunctionTraits<Starter>::ArgsTuple>;
    using Args = typename FunctionTraits<Callback>::ArgsTuple;
    if constexpr (std::is_function_v<Starter>)
        return detail::_CallbackAwaiterHelper_t<Starter*, Args>{ std::forward<Starter>(starter) };
    else return detail::_CallbackAwaiterHelper_t<Starter, Args>{ std::forward<Starter>(starter) };
}

LCORE_ASYNC_NAMESPACE_END
