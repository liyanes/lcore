#pragma once
#include "base.hpp"
#include "generator.hpp"
#include "traits.hpp"

LCORE_ASYNC_NAMESPACE_BEGIN

template <Iterable First, Iterable ...Rest>
Generator<std::tuple<RemoveReference<decltype(*std::declval<First>().begin())> , RemoveReference<decltype(*std::declval<Rest>().begin())>...>> _product(const First& first, const Rest&... rest){
    if constexpr (sizeof...(rest) == 0){
        for(auto f: first){
            co_yield std::make_tuple(f);
        }
    } else {
        for(auto f: first){
            auto pd = _product(rest...);
            for(auto r: pd){
                co_yield std::tuple_cat(std::make_tuple(f), r);
            }
        }
    }
};

template <Iterable ...Containers>
auto product(Containers&&... containers){
    return _product(std::forward<Containers>(containers)...);
}

LCORE_ASYNC_NAMESPACE_END
