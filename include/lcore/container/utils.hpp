#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <tuple>

LCORE_NAMESPACE_BEGIN

template <Iterable... Containers>
class zip {
public:
    struct sentinal {};
private:
    template <typename ...Iterators>
    class zip_iterator {
        std::tuple<std::pair<Iterators, Iterators>...> iters;
    public:
        zip_iterator(std::tuple<std::pair<Iterators, Iterators>...>&& iters): iters(std::move(iters)) {}

        inline zip_iterator operator++(){
            auto frommer = *this;
            std::apply([](auto&... iterargs){
                (iterargs.first++, ...);
            }, this->iters);
            return frommer;
        }

        inline zip_iterator& operator++(int){
            std::apply([](auto&... iterargs){
                (++iterargs.first, ...);
            }, this->iters);
            return *this;
        }

        inline bool operator==(sentinal){
            return std::apply([](auto&... iterargs){
                return ((iterargs.first == iterargs.second) || ...);
            }, this->iters);
        }

        template<size_t N>
        inline std::pair<NthType<N, Iterators...>, NthType<N, Iterators...>> GetCurrent(){
            return std::get<N>(this->iters);
        };

        inline std::tuple<RemoveReference<decltype(std::declval<Iterators>().operator*())>...> operator*(){
            return std::apply([](auto&&... iterargs){
                return std::make_tuple(
                    std::forward<RemoveReference<decltype(iterargs.first.operator*())>>(iterargs.first.operator*())...
                );
            }, this->iters);
        }
    };
public:
    using value_type = std::tuple<typename Containers::value_type...>;
    using iterator = zip_iterator<typename Containers::const_iterator...>;
    using const_iterator = zip_iterator<typename Containers::const_iterator...>;
    using reverse_iterator = zip_iterator<typename Containers::const_reverse_iterator...>;
    using const_reverse_iterator = zip_iterator<typename Containers::const_reverse_iterator...>;

    using view_type = zip<Containers...>;

private:
    std::tuple<const Containers*...> containers;

public:
    inline zip(const Containers&... containers): containers(&containers...){};

    inline iterator begin() {
        return iterator(std::apply([](auto... args){return std::make_tuple(std::make_pair(args->begin(), args->end())...);}, containers));
    }

    inline sentinal end() {
        return {};
    }

    inline const_iterator begin() const {
        return const_iterator(std::apply([](auto... args){return std::make_tuple(std::make_pair(args->cbegin(), args->cend())...);}, containers));
    }

    inline sentinal end() const {
        return {};
    }

    inline const_iterator cbegin() const {
        return const_iterator(std::apply([](auto... args){return std::make_tuple(std::make_pair(args->cbegin(), args->cend())...);}, containers));
    }

    inline sentinal cend() const {
        return {};
    }

    inline reverse_iterator rbegin() {
        return reverse_iterator(std::apply([](auto... args){return std::make_tuple(std::make_pair(args->rbegin(), args->rend())...);}, containers));
    }

    inline sentinal rend() {
        return {};
    }

    inline const_reverse_iterator rbegin() const {
        return const_reverse_iterator(std::apply([](auto... args){return std::make_tuple(std::make_pair(args->crbegin(), args->crend())...);}, containers));
    }

    inline sentinal rend() const {
        return {};
    }

    inline value_type operator[](size_t index) const {
        return std::apply([index](auto&... args){return std::make_tuple((*args)[index]...);}, containers);
    }

    inline view_type slice(size_t start, size_t end) const {
        return view_type(std::apply([start, end](auto... args){return std::make_tuple(args->slice(start, end)...);}, containers));
    }
};

LCORE_NAMESPACE_END
