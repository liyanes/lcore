#pragma once
#include "base.hpp"
#include "ref.hpp"
#include "lcore/container/view.hpp"

LCORE_REFLECTION_NAMESPACE_BEGIN

class Any;

/// @brief Function Reflection
class FunctionReflection: public ReflectionBase {
protected:
    using ReflectionBase::ReflectionBase;
public:
    struct Signature {
        TypeInfoPtr return_type;
        Span<TypeInfoPtr> param_types;
    };
    
    Signature signature;
    void(*invoke)(void* ret, void** args);
    void(*invoke_safe)(Any* ret, Span<Any> args);
};

LCORE_REFLECTION_NAMESPACE_END
