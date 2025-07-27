#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <coroutine>

LCORE_NAMESPACE_BEGIN

template <typename T>
concept HasCoAwaitOperator = requires(T t){
    {t.operator co_await()} -> IsSame<std::suspend_always>;
};

template <typename T>
concept __IsAwaitable = requires(T t){
    {t.await_ready()} -> std::same_as<bool>;
    {t.await_suspend()} -> std::same_as<std::coroutine_handle<>>;
    {t.await_resume()} -> std::same_as<typename T::value_type>;
};

template <typename T>
concept IsAwaitable = HasCoAwaitOperator<T> || __IsAwaitable<T>;

// Promise like

template <typename T>
concept PromiseLike = requires(T t){
    {t.get_return_object()} -> std::same_as<T>;
    {t.initial_suspend()} -> std::same_as<std::suspend_always>;
    {t.final_suspend()} -> std::same_as<std::suspend_always>;
    {t.unhandled_exception()};
    {t.return_value(std::declval<typename T::value_type>())};
};

LCORE_NAMESPACE_END
