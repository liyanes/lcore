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

LCORE_NAMESPACE_END
