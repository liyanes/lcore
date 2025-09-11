/**
 * @file enum.hpp
 * @brief Enum reflection utilities
 */

#pragma once
#include "base.hpp"
#include "ref.hpp"
#include "lcore/container.hpp"
#include "lcore/string.hpp"

LCORE_REFLECTION_NAMESPACE_BEGIN

class EnumReflection: public ReflectionBase {
protected:
    using ReflectionBase::ReflectionBase;
public:
    using EnumValueType = uint; // TODO: change to std::underlying_type_t<T> when T is known
    using EnumPairType = std::pair<StringView, EnumValueType>;

    StridedSpan<const StringView> names;
    StridedSpan<const EnumValueType> values;
    Span<const EnumPairType> entries;
};

/// @todo Implement dynamic enum reflection
// /// @brief Reflection for dynamic enums (enums that can be modified at runtime)
// class DynamicEnumReflection: public ReflectionBase {
// protected:
//     using ReflectionBase::ReflectionBase;
// public:
//     virtual bool HasName(StringView name) const = 0;
//     virtual PointerView<const StringView> Names() const = 0;
// };

LCORE_REFLECTION_NAMESPACE_END

