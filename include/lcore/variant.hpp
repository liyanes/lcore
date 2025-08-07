#pragma once
#include "base.hpp"
#include "traits.hpp"
#include <variant>

LCORE_NAMESPACE_BEGIN

/// @brief Type-safe variant
/// @tparam ...Args The types of the variant
template <typename ...Args>
class Variant: public std::variant<Args...> {
public:
    using std::variant<Args...>::variant;

    /// @brief Get the type of the union
    /// @return The type of the union
    inline const std::type_info& type() const {
        return GetNthType<Args...>(this->index());
    }

    /// @brief Test if the union is of a specific type
    /// @tparam T The type to be checked
    /// @return True if the union is of the type
    template <typename T>
    requires OneOf<T, Args...>
    inline bool is() const {
        return this->index() == IndexOf<T, Args...>;
    }

    /// @brief Get the value of the union if it is of a specific type
    /// @tparam T The type to be checked
    /// @return The value of the union if it is of the type
    template <typename T>
    requires OneOf<T, Args...>
    inline T& get() {
        return std::get<T>(*this);
    }

    /// @brief Get the value of the union if it is of a specific type
    /// @tparam T The type to be checked
    /// @return The value of the union if it is of the type
    template <typename T>
    requires OneOf<T, Args...>
    inline const T& get() const {
        return std::get<T>(*this);
    }
    
    template <typename T>
    requires OneOf<T, Args...>
    inline T getdefault(T value) const {
        if (is<T>()){
            return get<T>();
        }else{
            return value;
        }
    }
};

LCORE_NAMESPACE_END
