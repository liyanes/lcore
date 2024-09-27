#pragma once
#include <type_traits>
#include <tuple>
#include <cstddef>
#include "base.hpp"

LCORE_NAMESPACE_BEGIN

template <typename T>
concept Number = std::is_arithmetic_v<T>;

template <typename T>
concept Integer = std::is_integral_v<T>;

template <typename T>
concept Floating = std::is_floating_point_v<T>;

template <typename T>
concept Unsigned = std::is_unsigned_v<T>;

template <typename T>
concept Signed = std::is_signed_v<T>;

template <typename T>
concept Real = Floating<T> || Integer<T>;

template <typename T, typename U>
concept IsSame = std::is_same_v<T, U>;

template <typename T, typename ...Args>
concept IsAllSame = (IsSame<T, Args> && ...);

template <typename T, typename ...Items>
concept IsOneOf = (IsSame<T, Items> || ...);

template <typename T>
concept IsPointer = std::is_pointer_v<T>;

template <auto T, auto U>
concept IsEqual = IsSame<decltype(T), decltype(U)> && T == U;

template <auto T>
concept IsTrue = std::bool_constant<T>::value;

template <typename T>
using RemoveReference = std::remove_reference_t<T>;

template <typename Func, typename ...Args>
concept IsCallable = requires(Func func, Args... args){
    { func(args...) };
};

template <typename Func, typename ...Args>
using ResultCallable = decltype(std::declval<Func>()(std::declval<Args>()...));

template <typename T>
concept IsConst = std::is_const_v<T>;

/**
 * Iterator Traits, see also in std::ranges::iterator_traits
*/

/// @brief Concept for checking if a type is iterable
/// @tparam T The type to be checked
template <typename T>
concept Iterable = requires(T t){
    t.begin();
    t.end();
    t.begin() == t.end();
    t.begin() != t.end();
};

/// @brief Get the value type of an iterable
/// @tparam T The iterable type
template <typename T>
using IterableValueType = RemoveReference<decltype(*std::declval<T>().begin())>;

template <typename T>
concept ConstIterable = requires(const T t){
    Iterable<T>;
    IsConst<IterableValueType<T>>;
};


/// @brief Concept for checking if a type is a map
/// @tparam T The type to be checked
template <typename T>
concept IsMap = requires(T t){
    Iterable<T>;
    {IterableValueType<T>::first} -> std::same_as<typename T::key_type>;
    {IterableValueType<T>::second} -> std::same_as<typename T::mapped_type>;
};

/// @brief Get the key type of a map
/// @tparam T The map type
template <typename T>
using MapKeyType = typename T::key_type;

/// @brief Get the value type of a map
/// @tparam T The map type
template <typename T>
using MapValueType = typename T::mapped_type;

template <typename Func, typename ...Args>
using ReturnType = decltype(std::declval<Func>()(std::declval<Args>()...));

template <typename T>
concept StringLike = requires(T t){
    Iterable<T>;
    IsSame<IterableValueType<T>, char>;
    {t.c_str()} -> IsOneOf<const char*, const char[]>;
    {t.size()} -> IsSame<size_t>;
};

template <typename T>
concept IsStandardLayout = std::is_standard_layout_v<T>;

template <typename T>
concept IsTrivial = std::is_trivial_v<T>;

template <typename T>
concept IsPOD = IsStandardLayout<T> && IsTrivial<T>;

/// @brief Simple type (not class or pointer or reference)
/// @tparam T The type to be checked
template <typename T>
concept SimpleType = !std::is_class_v<T> && !std::is_pointer_v<T> && !std::is_reference_v<T>;

/// @brief Enum type
/// @tparam T The type to be checked
template <typename T>
concept IsEnum = std::is_enum_v<T>;

/// @brief Underlying type of an enum
/// @tparam T The enum type
template <typename T>
using UnderlyingType = std::underlying_type_t<T>;

template <typename T, typename Base>
concept IsDerivedFrom = std::is_base_of_v<Base, T>;

template <typename T, typename U>
concept CanConvTo = std::is_convertible_v<T, U>;

#define GetMemberType(ParentType, MemberName) (decltype(std::declval<ParentType>.##MemberName))

/// @brief Get the Nth type of a type list
/// @tparam index The index of the type
/// @tparam Args The type list
template <size_t index, typename ...Args>
using NthType = std::tuple_element_t<index, std::tuple<Args...>>;

template <size_t index, typename Tuple>
using NthTypeOfTuple = std::tuple_element_t<index, Tuple>;


template <typename T, typename ...Args>
struct _IndexOf;

template <typename T, typename ...Args>
struct _IndexOf<T, T, Args...>: std::integral_constant<size_t, 0> {};

template <typename T, typename U, typename ...Args>
struct _IndexOf<T, U, Args...>: std::integral_constant<size_t, 1 + _IndexOf<T, Args...>::value> {};

/// @brief Get the index of a type in a type list
/// @tparam T The type to be found
/// @tparam Args The type list
template <typename T, typename ...Args>
inline constexpr size_t IndexOf = _IndexOf<T, Args...>::value;

template <typename T, typename U>
concept Comparable = requires(T t, U u){
    {t == u} -> IsSame<bool>;
    {t != u} -> IsSame<bool>;
    {t < u} -> IsSame<bool>;
    {t > u} -> IsSame<bool>;
    {t <= u} -> IsSame<bool>;
    {t >= u} -> IsSame<bool>;
};

template <typename T, typename U>
concept EqualComparable = requires(T t, U u){
    {t == u} -> IsSame<bool>;
    {t != u} -> IsSame<bool>;
};

template <bool Cond, typename T = void>
using EnableIf = std::enable_if_t<Cond, T>;

template <bool Cond, typename T, typename F>
using Conditional = std::conditional_t<Cond, T, F>;

template <typename T>
concept DefaultConstructible = std::is_default_constructible_v<T>;

template <typename T>
concept CopyConstructible = std::is_copy_constructible_v<T>;

template <typename T>
concept MoveConstructible = std::is_move_constructible_v<T>;

template <typename T>
concept CopyAssignable = std::is_copy_assignable_v<T>;

template <typename T>
concept MoveAssignable = std::is_move_assignable_v<T>;

template <typename T>
concept Destructible = std::is_destructible_v<T>;

// RemoveNthOfTuple
template <size_t index, typename Tuple>
struct _RemoveNthOfTuple;

template <size_t index, typename ...Args>
struct _RemoveNthOfTuple<index, std::tuple<Args...>>{
    using type = typename _RemoveNthOfTuple<index - 1, std::tuple<Args...>>::type;
};

template <typename First, typename ...Args>
struct _RemoveNthOfTuple<0, std::tuple<First, Args...>>{
    using type = std::tuple<Args...>;
};

template <size_t index, typename Tuple>
using RemoveNthOfTuple = typename _RemoveNthOfTuple<index, Tuple>::type;

#ifdef LCORE_TRAITS_USE_COROUTINE
#include <coroutine>

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
#endif

LCORE_NAMESPACE_END