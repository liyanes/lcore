#include "../container.hpp"

LCORE_NAMESAPCE_BEGIN

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

// /// @brief Product of multiple containers
// /// @details This class is used to iterate over the product of multiple containers
// /// @tparam ...Containers 
// template <typename ...Containers>
// class product {
// public:
//     struct sentinal {};
//     using value_type = std::tuple<typename Containers::value_type...>;
// private:
//     using _subiters_begintype = std::tuple<decltype(std::declval<Containers>().cbegin())...>;
//     using _subiters_endtype = std::tuple<decltype(std::declval<Containers>().cend())...>;
    
//     using _first_begin = NthTypeOfTuple<0, _subiters_begintype>;
//     using _first_end = NthTypeOfTuple<0, _subiters_endtype>;
    
//     using _left_containers_tps = RemoveNthOfTuple<0, std::tuple<const Containers*...>>;

//     template <typename Tps, size_t ...Indexes>
//     inline auto static _product_factory(Tps tps, std::index_sequence<Indexes...>){
//         return product(std::get<Indexes>(tps)...);
//     }

//     using _left_product = decltype(_product_factory(std::declval<_left_containers_tps>(), std::make_index_sequence<sizeof...(Containers) - 1>{}));

// public:
//     class iterator {
//         _first_begin first_begin;
//         _first_end first_end;

//         _left_product left_product;
//         typename _left_product::iterator left_iter;
//     public:
//         inline iterator(_first_begin first_begin, _first_end first_end, _left_product left_product): first_begin(first_begin), first_end(first_end), left_product(left_product), left_iter(left_product.begin()) {}

//         inline iterator operator++(){
//             auto frommer = *this;
//             if (++left_iter == left_product.end()){
//                 left_iter = left_product.begin();
//                 ++first_begin;
//             }
//             return frommer;
//         }

//         inline iterator& operator++(int){
//             if (++left_iter == left_product.end()){
//                 left_iter = left_product.begin();
//                 ++first_begin;
//             }
//             return *this;
//         }

//         inline bool operator==(sentinal){
//             return first_begin == first_end;
//         }

//         inline value_type operator*(){
//             return std::tuple_cat(std::make_tuple(*first_begin), *left_iter);
//         }
//     };

//     using const_iterator = iterator;
//     using view_type = product<const Containers*...>;

// private:
//     std::tuple<const Containers*...> containers;
// public:
//     inline product(const Containers&... containers): containers(&containers...) {}

//     inline iterator begin() {
//         return iterator(std::apply([](auto... args){return std::make_tuple(args->cbegin()...);}, containers), std::apply([](auto... args){return std::make_tuple(args->cend()...);}, containers), std::apply([](auto... args){return product(args...);}, containers));
//     }

//     inline sentinal end() {
//         return {};
//     }

//     inline const_iterator begin() const {
//         return iterator(std::apply([](auto... args){return std::make_tuple(args->cbegin()...);}, containers), std::apply([](auto... args){return std::make_tuple(args->cend()...);}, containers), std::apply([](auto... args){return product(args...);}, containers));
//     }

//     inline sentinal end() const {
//         return {};
//     }

//     inline const_iterator cbegin() const {
//         return iterator(std::apply([](auto... args){return std::make_tuple(args->cbegin()...);}, containers), std::apply([](auto... args){return std::make_tuple(args->cend()...);}, containers), std::apply([](auto... args){return product(args...);}, containers));
//     }

//     inline sentinal cend() const {
//         return {};
//     }

//     inline view_type slice(size_t start, size_t end) const {
//         return view_type(std::apply([start, end](auto... args){return std::make_tuple(args->slice(start, end)...);}, containers));
//     }
// };


LCORE_NAMESAPCE_END
