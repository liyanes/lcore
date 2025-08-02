#pragma once
#include "base.hpp"
#include "../container.hpp"
#include "../pointer.hpp"
#include "../string.hpp"
#include "../class.hpp"
#include "../exception.hpp"
#include <map>
#include <functional>

LCORE_OBJ_NAMESPACE_BEGIN

class TypeInfo;
class Object;

class TypeId: public RawPtr<const TypeInfo>{
public:
    TypeId(const TypeInfo* type): RawPtr<const TypeInfo>(type){};
    TypeId(const TypeId&) = default;
    TypeId(TypeId&&) = default;
    TypeId& operator=(const TypeId&) = default;
    TypeId& operator=(TypeId&&) = default;

    inline Ptr<Object> Constructe() const;
};

class TypeInfo {
public:
    const StringView name;
    const TypeId parent;
    std::function<Ptr<Object>()> constructor;
};

inline Ptr<Object> TypeId::Constructe() const{
    return (*this)->constructor();
}

LCORE_OBJ_NAMESPACE_END

