#pragma once
#include "base.hpp"
#include "lcore/class.hpp"
#include "lcore/traits.hpp"
#include "lcore/string.hpp"
#include "lcore/pointer.hpp"
#include <map>

LCORE_REFLECTION_NAMESPACE_BEGIN

using TypeInfo = std::type_info;
using TypeInfoRef = const TypeInfo&;
using TypeInfoPtr = RawPtr<const TypeInfo>;

class ReflectionBase: public AbstractClass {
public:
    const TypeInfoPtr type;
    const std::size_t size;
    const std::size_t align;
protected:
    inline ReflectionBase(TypeInfoPtr t, std::size_t size, std::size_t align)
        : type(t), size(size), align(align) {}
public:
    ReflectionBase(const ReflectionBase&) = delete;
    ReflectionBase(ReflectionBase&&) = delete;
    ReflectionBase& operator=(const ReflectionBase&) = delete;
    ReflectionBase& operator=(ReflectionBase&&) = delete;

    virtual TypeInfoRef GetReflectionType() const = 0;
    virtual String Serialize(const void* instance) const = 0;
};

class ReflectionSet {
    std::map<const std::type_info*, const ReflectionBase*> reflections_;
public:
    inline ReflectionSet() = default;
    inline ~ReflectionSet() = default;

    inline void Add(const ReflectionBase* reflection) {
        reflections_.emplace(reflection->type, reflection);
    }

    inline const ReflectionBase* Get(const std::type_info& type) const {
        auto it = reflections_.find(&type);
        if (it != reflections_.end()) {
            return it->second;
        }
        return nullptr;
    }
};

RawPtr<ReflectionSet> GlobalReflectionSet();

template <typename T>
RawPtr<const ReflectionBase> GetReflection() {
    return GlobalReflectionSet()->Get(typeid(T));
};

LCORE_REFLECTION_NAMESPACE_END
