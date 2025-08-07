#pragma once
#include "base.hpp"
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

template <typename T>
struct __EnumComparable {
    static constexpr bool value = false;
};

#define LCORE_ENUM_COMPARABLE(T) \
    template <> \
    struct __EnumComparable<T> { \
        static constexpr bool value = true; \
    };

template <Enum EnumT>
requires __EnumComparable<EnumT>::value
inline static constexpr bool operator==(EnumT lhs, UnderlyingType<EnumT> rhs) {
    return static_cast<UnderlyingType<EnumT>>(lhs) == rhs;
}

template <Enum EnumT>
requires __EnumComparable<EnumT>::value
inline static constexpr bool operator!=(EnumT lhs, UnderlyingType<EnumT> rhs) {
    return static_cast<UnderlyingType<EnumT>>(lhs) != rhs;
}

template <typename T>
struct __EnumBitwiseOperators {
    static constexpr bool value = false;
};

#define LCORE_ENUM_BITWISE_OPERATORS(T) \
    template <> \
    struct __EnumBitwiseOperators<T> { \
        static constexpr bool value = true; \
    };

template <Enum EnumT>
requires __EnumBitwiseOperators<EnumT>::value
inline static constexpr EnumT operator|(EnumT lhs, EnumT rhs) {
    return static_cast<EnumT>(
        static_cast<UnderlyingType<EnumT>>(lhs) |
        static_cast<UnderlyingType<EnumT>>(rhs)
    );
}

template <Enum EnumT>
requires __EnumBitwiseOperators<EnumT>::value
inline static constexpr EnumT operator&(EnumT lhs, EnumT rhs) {
    return static_cast<EnumT>(
        static_cast<UnderlyingType<EnumT>>(lhs) &
        static_cast<UnderlyingType<EnumT>>(rhs)
    );
}

template <Enum EnumT>
requires __EnumBitwiseOperators<EnumT>::value
inline static constexpr EnumT operator^(EnumT lhs, EnumT rhs) {
    return static_cast<EnumT>(
        static_cast<UnderlyingType<EnumT>>(lhs) ^
        static_cast<UnderlyingType<EnumT>>(rhs)
    );
}

template <Enum EnumT>
requires __EnumBitwiseOperators<EnumT>::value
inline static constexpr EnumT operator~(EnumT value) {
    return static_cast<EnumT>(
        ~static_cast<UnderlyingType<EnumT>>(value)
    );
}

LCORE_NAMESPACE_END
