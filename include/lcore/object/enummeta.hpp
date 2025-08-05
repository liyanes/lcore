#pragma once
#include "base.hpp"
#include "lcore/container/vector.hpp"

LCORE_OBJ_NAMESPACE_BEGIN

template <typename BaseType, typename EnumT, typename InfoDetail = std::default_sentinel_t>
class EnumMeta;

template <typename BaseType, typename InfoDetail = std::default_sentinel_t>
class EnumMetaBase {
    const std::type_info* m_typeInfo;

    static List<EnumMetaBase*>& GetAllEnumMetas() {
        static List<EnumMetaBase*> m_allEnumMetas;
        return m_allEnumMetas;
    }
public:
    /// @brief EnumInfo is a structure that holds information about an enum value
    struct EnumInfo {
        const char* name;
        InfoDetail detail; // Additional information about the enum value
    };
    using Enum = EnumInfo*;
    
    template <typename T>
    using MetaType = EnumMeta<BaseType, T, InfoDetail>;
    
    template <typename T>
    static MetaType<T> NewMeta() {
        return MetaType<T>();
    }
protected:
    Vector<EnumInfo> m_values;
    static void Register(EnumMetaBase* meta) { GetAllEnumMetas().push_back(meta); }
    EnumMetaBase(const std::type_info* typeInfo) : m_typeInfo(typeInfo) { Register(this); }
public:
    EnumMetaBase(const EnumMetaBase&) = delete;
    EnumMetaBase& operator=(const EnumMetaBase&) = delete;

    static EnumMetaBase* FindByType(const std::type_info& typeInfo) {
        for (auto& meta : GetAllEnumMetas()) {
            if (*meta->m_typeInfo == typeInfo) {
                return meta;
            }
        }
        return nullptr;
    }
    template <typename T>
    static EnumMetaBase* FindByType() { return FindByType(typeid(T)); }

    inline bool operator==(const EnumMetaBase& other) const { return m_typeInfo == other.m_typeInfo; }
    inline bool operator!=(const EnumMetaBase& other) const { return m_typeInfo != other.m_typeInfo; }

    Vector<EnumInfo>::const_iterator begin() const { return m_values.begin(); }
    Vector<EnumInfo>::const_iterator end() const { return m_values.end(); }
    Vector<const char*> GetValues() const {
        Vector<const char*> values;
        values.reserve(m_values.size());
        for (const auto& value : m_values) {
            values.push_back(value.name);
        }
        return values;
    }
    Vector<EnumInfo>::const_iterator GetValueInfo(StringView name) const {
        return std::find_if(m_values.begin(), m_values.end(),
            [&name](const EnumInfo& info) { return name == info.name; });
    }
};

template <typename BaseType, typename EnumT, typename InfoDetail>
class EnumMeta final: public EnumMetaBase<BaseType, InfoDetail> {
public:
    using EnumInfo = typename EnumMetaBase<BaseType, InfoDetail>::EnumInfo;
    using Enum = typename EnumMetaBase<BaseType, InfoDetail>::Enum;

    EnumMeta() : EnumMetaBase<BaseType, InfoDetail>(&typeid(EnumT)) {}

    Enum AddValue(const char* name, InfoDetail detail = InfoDetail{}) {
        EnumInfo info{name, detail};
        this->m_values.push_back(info);
        return &this->m_values.back();
    }
    Enum AddValue(StringView name, InfoDetail detail = InfoDetail{}) {
        return AddValue(name.data(), detail);
    }
};

/**
 * How to use EnumMeta:
 * ```cpp
 * struct MyEnumBase {};
 * template <typename T>
 * class MyEnum: public EnumMeta<MyEnumBase, T> {};
 * // In else .cpp file:
 * // List<MyEnumBase*> MyEnumBase::m_allEnumMetas;
 * 
 * struct MyFirstEnum: MyEnumBase {
 *     static MyEnum<MyFirstEnum> value1, 
 *           value2;  
 * };
 * auto MyFirstEnumMeta = MyEnum<MyFirstEnum>::NewMeta();
 * MyFirstEnum::value1 = MyFirstEnumMeta.AddValue("Value1", "This is the first enum value");
 * MyFirstEnum::value2 = MyFirstEnumMeta.AddValue("Value2", "This is the second enum value");
 * ```
 */

LCORE_OBJ_NAMESPACE_END
