#pragma once
#include "structmeta.hpp"
#include "../variant.hpp"
#include <map>
#include <typeindex>

LCORE_OBJ_NAMESPACE_BEGIN

template <typename BaseType, typename NameType = StringView>
class StructKeyValueIterator {
public:
    using StructMetaBase = StructMetaBase<BaseType, NameType>;
    using StructKeyInfo = typename StructMetaBase::StructKeyInfo;

    using sentinel = std::default_sentinel_t;
private:
    StructMetaBase* meta;
    void* structPtr;
    Vector<StructKeyInfo>::const_iterator it;
public:
    using ValueVariant = Variant<
        // String type
        const char*, String, StringView,
        // Numeric types
        int, unsigned, long, unsigned long, long long, unsigned long long,
        float, double,
        // Pointer types
        void*, const void*,
        // Struct type
        StructKeyValueIterator<BaseType, NameType>
    >;
    using ValueType = std::pair<
        StructKeyInfo, ValueVariant
    >;

    StructKeyValueIterator(StructMetaBase* meta, void* structPtr)
        : meta(meta), structPtr(structPtr), it(meta->begin()) {}

    bool operator!=(const StructKeyValueIterator& other) const {
        return it != other.it;
    }

    bool operator!=(sentinel) const {
        return it != meta->end();
    }

    StructKeyValueIterator& operator++() {
        ++it;
        return *this;
    }

    ValueType operator*() const {
        if (it == meta->end()) throw std::out_of_range("Iterator out of range");
        const StructKeyInfo& keyInfo = *it;
        static std::map<std::type_index, ValueVariant(const StructKeyInfo&, void*)> typeHandlers = {
            {typeid(String), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(String(reinterpret_cast<char*>(structPtr) + keyInfo.offset, keyInfo.size));
            }},
            {typeid(StringView), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(StringView(reinterpret_cast<char*>(structPtr) + keyInfo.offset, keyInfo.size));
            }},
            {typeid(int), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<int*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(unsigned), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<unsigned*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(long), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<long*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(unsigned long), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<unsigned long*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(long long), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<long long*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(unsigned long long), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<unsigned long long*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(float), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<float*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(double), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(*reinterpret_cast<double*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(void*), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(reinterpret_cast<void*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(const void*), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return ValueVariant(reinterpret_cast<const void*>(reinterpret_cast<char*>(structPtr) + keyInfo.offset));
            }},
            {typeid(StructKeyValueIterator<BaseType, NameType>), [](const StructKeyInfo& keyInfo, void* structPtr) {
                return StructKeyValueIterator<BaseType, NameType>(keyInfo.structMeta, reinterpret_cast<char*>(structPtr) + keyInfo.offset);
            }}
        };
        auto it = typeHandlers.find(keyInfo.typeInfo->hash_code());
        if (it == typeHandlers.end()) {
            throw std::runtime_error("Unsupported type in StructKeyValueIterator");
        }
        return {keyInfo, it->second(keyInfo, structPtr)};
    }
};

LCORE_OBJ_NAMESPACE_END
