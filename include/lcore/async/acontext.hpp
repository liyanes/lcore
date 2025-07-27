#pragma once
#include "base.hpp"
#include "traits.hpp"
#include "task.hpp"

LCORE_ASYNC_NAMESPACE_BEGIN

template <typename InitRet, typename ...InitArgs>
class AsyncContext: AbstractClass {
protected:
    virtual Task<InitRet> DoInit(InitArgs&& ...args) = 0;
    virtual Task<void> DoFinal() = 0;
public:
    template <typename Func>
    requires Awaitable<ResultCallable<Func, InitRet>>
    Task<void> Enter(InitArgs&& ...args, Func func){
        Task<InitRet> init = DoInit(std::forward<InitArgs>(args)...);
        co_await init;
        co_await func(init.get());
        co_await DoFinal();
        co_return;
    }
};

LCORE_ASYNC_NAMESPACE_END
