#pragma once
#include "base.hpp"
#include "lcore/container/list.hpp"
#include "lcore/container/vector.hpp"
#include "lcore/string.hpp"

LCORE_OBJ_NAMESPACE_BEGIN

template <typename BaseType, typename T, typename NameType = StringView>
class StructMeta;

/// @brief StructMetaBase is a base class for struct metadata that provides a way to define and manage metadata for structs.
/// @tparam BaseType An identifier for the base type of the struct. This is used to differentiate between different struct metadata types.
template <typename BaseType, typename NameType = StringView>
class StructMetaBase {
    static List<StructMetaBase*> m_allStructMetas;
    const std::type_info* m_typeInfo;
    const size_t m_size;
public:
    using BaseType = BaseType;
    using NameType = NameType;

    /// @brief StructKeyInfo is a structure that holds information about a key in the struct
    struct StructKeyInfo {
        NameType name;
        size_t offset;
        size_t size;
        const std::type_info* typeInfo;
        const StructMetaBase* structMeta; // If this is a struct, this will be the meta of the struct
    };
protected:
    Vector<StructKeyInfo> m_keys;
    
    StructMetaBase(const std::type_info* typeInfo, size_t size) : m_typeInfo(typeInfo), m_size(size) { Register(this); }
    static void Register(StructMetaBase* meta) { m_allStructMetas.push_back(meta); }
public:
    StructMetaBase(const StructMetaBase&) = delete;
    StructMetaBase& operator=(const StructMetaBase&) = delete;

    static StructMetaBase* FindByType(const std::type_info& typeInfo) {
        for (auto& meta : m_allStructMetas) {
            if (*meta->m_typeInfo == typeInfo) {
                return meta;
            }
        }
        return nullptr;
    }
    template <typename T>
    static StructMetaBase* FindByType() { return FindByType(typeid(T)); }

    inline bool operator==(const StructMetaBase& other) const { return m_typeInfo == other.m_typeInfo; }
    inline bool operator!=(const StructMetaBase& other) const { return m_typeInfo != other.m_typeInfo; }
    
    std::size_t GetSize() const { return m_size; }
    
    Vector<StructKeyInfo>::const_iterator begin() const { return m_keys.begin(); }
    Vector<StructKeyInfo>::const_iterator end() const { return m_keys.end(); }
    Vector<NameType> GetKeys() const {
        Vector<NameType> keys;
        keys.reserve(m_keys.size());
        for (const auto& key : m_keys) {
            keys.push_back(key.name);
        }
        return keys;
    }
    Vector<StructKeyInfo>::const_iterator GetKeyInfo(NameType key) const {
        return std::find_if(m_keys.begin(), m_keys.end(),
            [&key](const StructKeyInfo& info) { return info.name == key; });
    }

    template <typename T>
    using MetaType = StructMeta<BaseType, T, NameType>;

    template <typename T>
    static MetaType<T> NewMeta() { return MetaType<T>(); }
};

/// @brief StructMeta is a template class that provides metadata for a struct type.
/// @note You must hold this object to access the metadata of the struct. Do not release it until you are done with the metadata.
template <typename BaseType, typename T, typename NameType>
class StructMeta final: public StructMetaBase<BaseType, NameType> {
    static_assert(std::is_trivially_copyable_v<T>, "StructMeta can only be used with trivially copyable types");
    static_assert(!std::is_pointer_v<T>, "StructMeta cannot be used with pointer types");
    static_assert(std::derived_from<T, BaseType>, "StructMeta can only be used with types that derive from BaseType");

    using StructType = T;
public:
    StructMeta() : StructMetaBase(&typeid(T), sizeof(T)) {}

    template <typename KeyType>
    requires std::is_trivially_copyable_v<KeyType>
    StructMeta& AddKey(NameType name, size_t offset) {
        m_keys.push_back({name, offset, sizeof(KeyType), &typeid(KeyType), nullptr});
        return this;
    }

    template <typename StructType>
    requires std::is_class_v<StructType> && std::is_trivially_copyable_v<StructType>
    StructMeta& AddStructKey(NameType name, size_t offset) {
        auto structMeta = StructMetaBase::FindByType(typeid(StructType));
        if (!structMeta) throw;
        m_keys.push_back({name, offset, sizeof(StructType), &typeid(StructType), structMeta});
        return this;
    }
};

/**
 * How to use StructMeta:
 * ```cpp
 * struct MyStructBase {}; // Used to identify the base type
 * template <typename T>
 * using MyStructMeta = StructMeta<MyStructBase, T>;
 * // In some .cpp file
 * // List<StructMetaBase*> MyStructMeta::m_allStructMetas;
 * 
 * struct MyStruct: MyStructBase {
 *    int a;
 *    float b;
 *    unsigned char c;
 * };
 * // Register the struct metadata
 * StructMeta<MyStructBase, MyStruct> myStructMeta;
 * myStructMeta.AddKey<int>("a", offsetof(MyStruct, a), sizeof(int));
 * myStructMeta.AddKey<float>("b", offsetof(MyStruct, b), sizeof(float));
 * myStructMeta.AddKey<unsigned char>("c", offsetof(MyStruct, c), sizeof(unsigned char));
 * 
 * // When you need to access the metadata:
 * auto meta = MyStructMeta::FindByType<MyStruct>();
 */


LCORE_OBJ_NAMESPACE_END
