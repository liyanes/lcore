#pragma once
#include "base.hpp"
#include "lcore/container/list.hpp"
#include "lcore/container/vector.hpp"
#include "lcore/string.hpp"
#include "lcore/traits.hpp"
#include <typeindex>

LCORE_OBJ_NAMESPACE_BEGIN

template <typename MarkType, typename T, typename NameType = StringView>
requires StandardLayout<T>
class StructMeta;

/// @brief StructMetaBase is a base class for struct metadata that provides a way to define and manage metadata for structs.
/// @tparam MarkType An identifier for the mark type of the struct. This is used to differentiate between different struct metadata types.
/// @note This class is only available for Standard Layout types.
template <typename MarkT, typename NameT = StringView>
class StructMetaBase {
    const std::type_info* m_typeInfo;
    const size_t m_size;

    static List<StructMetaBase*>& GetAllStructMetas() {
        static List<StructMetaBase*> allStructMetas;
        return allStructMetas;
    };
public:
    using MarkType = MarkT;
    using NameType = NameT;

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
    void (*CopyConstruct)(void* dest, const void* src) = nullptr;
    void (*Destruct)(void* obj) = nullptr;
    
    static void Register(StructMetaBase* meta) { 
        GetAllStructMetas().push_back(meta); 
    }
    StructMetaBase(const std::type_info* typeInfo, size_t size) : m_typeInfo(typeInfo), m_size(size) { Register(this); }

    void SetCopyConstruct(void (*func)(void* dest, const void* src)) { this->CopyConstruct = func; }
    void SetDestruct(void (*func)(void* obj)) { this->Destruct = func; }
public:
    StructMetaBase(const StructMetaBase&) = delete;
    StructMetaBase& operator=(const StructMetaBase&) = delete;
    // Register(this), `this` is incompatible with move constructor
    StructMetaBase(StructMetaBase&&) = delete;
    StructMetaBase& operator=(StructMetaBase&&) = delete;

    static StructMetaBase* FindByType(const std::type_info& typeInfo) {
        for (auto& meta : GetAllStructMetas()) {
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
    void* CopyConstructObj(void* dest, const void* src) const {
        if (this->CopyConstruct) {
            return this->CopyConstruct(dest, src);
        }
        return nullptr;
    }
    void DestructObj(void* obj) const {
        if (this->Destruct) {
            this->Destruct(obj);
        }
    }

    template <typename T>
    using MetaType = StructMeta<MarkType, T, NameType>;

    template <typename T>
    static MetaType<T> NewMeta() { return MetaType<T>(); }
};

/// @brief StructMeta is a template class that provides metadata for a struct type.
/// @note You must hold this object to access the metadata of the struct. Do not release it until you are done with the metadata.
template <typename MarkType, typename T, typename NameType>
requires StandardLayout<T>
class StructMeta final: public StructMetaBase<MarkType, NameType> {
    static_assert(std::is_copy_constructible_v<T>, "StructMeta can only be used with trivially copyable types");
    static_assert(!std::is_pointer_v<T>, "StructMeta cannot be used with pointer types");

    using StructType = T;
public:
    StructMeta() : StructMetaBase<MarkType, NameType>(&typeid(T), sizeof(T)) {
        this->SetCopyConstruct([](void* dest, const void* src) {
            dest = new T(*reinterpret_cast<const T*>(src));
        });
        this->SetDestruct([](void* obj) {
            delete reinterpret_cast<T*>(obj);
        });
    }

    template <typename KeyType>
    requires std::is_copy_constructible_v<KeyType>
    StructMeta& AddKey(NameType name, size_t offset) {
        this->m_keys.push_back({name, offset, sizeof(KeyType), &typeid(KeyType), nullptr});
        return *this;
    }

    template <typename MemberType>
    requires std::is_member_object_pointer_v<MemberType>
    StructMeta& AddKey(NameType name, MemberType member) {
        using MemberT = std::remove_cvref_t<ExtractMemberType<MemberType>>;
        static_assert(std::is_copy_constructible_v<MemberT>, "Member type must be copy constructible");
        return AddKey<MemberT>(name, reinterpret_cast<uintptr_t>(&(static_cast<T*>(nullptr)->*member)));
    }

    template <typename StructType>
    requires std::is_class_v<StructType> && std::is_copy_constructible_v<StructType>
    StructMeta& AddStructKey(NameType name, size_t offset) {
        auto structMeta = StructMetaBase<MarkType, NameType>::FindByType(typeid(StructType));
        if (!structMeta) throw;
        this->m_keys.push_back({name, offset, sizeof(StructType), &typeid(StructType), structMeta});
        return *this;
    }
};

/**
 * How to use StructMeta:
 * ```cpp
 * template <typename T>
 * using MyStructMeta = StructMeta<decltype(struct MyStructMark {}), T>;
 * 
 * struct MyStruct {
 *    int a;
 *    float b;
 *    unsigned char c;
 * };
 * // Register the struct metadata
 * static MyStructMeta::MetaType<MyStruct> myStructMeta = MyStructMeta::NewMeta<MyStruct>();
 * static auto _ = [] {
 *     myStructMeta.AddKey<int>("a", offsetof(MyStruct, a));
 *     myStructMeta.AddKey<float>("b", offsetof(MyStruct, b));
 *     myStructMeta.AddKey<unsigned char>("c", offsetof(MyStruct, c));
 *     return 0;
 * };
 * 
 * // When you need to access the metadata:
 * auto meta = MyStructMeta::FindByType<MyStruct>();
 */


LCORE_OBJ_NAMESPACE_END
