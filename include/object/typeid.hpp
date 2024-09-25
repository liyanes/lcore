#include "../config.h"
#include "../container.hpp"
#include "../pointer.hpp"
#include "../string.hpp"
#include "../class.hpp"
#include "../exception.hpp"
#include <map>

LCORE_NAMESAPCE_BEGIN

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

    inline bool operator==(const TypeId& other) const noexcept {
        return this->ptr == other.ptr;
    }
    inline bool operator==(nullptr_t) const noexcept {
        return this->ptr == nullptr;
    }
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

LCORE_NAMESAPCE_END

