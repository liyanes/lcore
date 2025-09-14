/**
 * @file base.hpp
 * @author liyanes(liyanes@outlook.com)
 * @brief Base definitions for the library
 * @version 0.1
 * @date 2024-06-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#if __cplusplus < 202002L
#error "This library requires C++20 or later"
#endif

// #define LCORE_NAMESPACE_BEGIN namespace lcore {
// #define LCORE_NAMESPACE_END }
#include "config.h"

#include "traits.hpp"
#include "rawptr.hpp"
#include "class.hpp"

LCORE_NAMESPACE_BEGIN

template <typename T>
using Ref = T&;

using TypeInfo = std::type_info;
using TypeInfoPtr = RawPtr<const TypeInfo>;
using TypeInfoRef = Ref<const TypeInfo>;

LCORE_NAMESPACE_END
