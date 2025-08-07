#pragma once
#if !__has_include(<fmt/format.h>)
#error "fmt library is required for this header"
#else

#include "base.hpp"
#include "type.hpp"
#include "traits.hpp"
#include <vector>

LCORE_NAMESPACE_BEGIN


/// @brief Get the string representation of a value
/// @tparam T The type of the value
/// @param value The value
/// @return The string representation of the value
template <typename T>
inline std::string repr(T&& value){
    if constexpr (DerivedFrom<std::decay_t<T>, std::string>){
        return (std::stringstream() << std::quoted(std::forward<T>(value))).str();
    }else if constexpr (DerivedFrom<std::decay_t<T>, std::string_view>){
        return (std::stringstream() << std::quoted(std::string(value))).str();
    }else if constexpr (fmt::has_formatter<std::decay_t<T>, fmt::format_context>::value){
        return fmt::format("{}", std::forward<T>(value));
    }else if constexpr (Pointer<std::decay_t<T>>){
        if (value == nullptr) return "<nullptr>";
        return fmt::format("<{}->{}>", (void*)value, repr(*value));
    }else if constexpr (IsMap<std::decay_t<T>>){
        return fmt::format("{{{}}}", Utils::JoinAsString(value | std::ranges::views::transform([](auto&& v){return fmt::format("{}: {}", repr(v.first), repr(v.second));})));
    }else if constexpr (ConstIterable<std::decay_t<T>>){
        if (value.begin() == value.end()) return "[]";
        return fmt::format("[{}]", Utils::JoinAsString(value | std::ranges::views::transform([](auto&& v){return repr(v);})));
    }else{
        return fmt::format("<{} {}>", demangle<T>(), (void*)&value);
    }
}

/// @brief Get the string representation of a list of values
/// @tparam ...Args The types of the values
/// @param ...args The values
/// @return The string representation of the values
template <typename ...Args>
inline std::vector<std::string> repr(Args&&... args){
    return std::vector<std::string>{repr(std::forward<Args>(args))...};
}


LCORE_NAMESPACE_END

template <typename T>
struct fmt::formatter<std::shared_ptr<T>>: public fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const std::shared_ptr<T>& p, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "<S{}->{}>", (void*)p.get(), Compiler::repr(*p));
    }
};

#endif
