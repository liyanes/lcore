#pragma once
#include "base.hpp"
#include "lcore/container/list.hpp"

LCORE_OBJ_NAMESPACE_BEGIN

template <typename BaseType, typename EnumT, typename InfoDetail = std::default_sentinel_t>
class EnumMeta;

template <typename BaseType, typename InfoDetail = std::default_sentinel_t>
class EnumMetaBase {
    static List<EnumMetaBase*> m_allEnumMetas;
    const std::type_info* m_typeInfo;
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
    List<EnumInfo> m_values;
    EnumMeta(const std::type_info* typeInfo) : m_typeInfo(typeInfo) { Register(this); }
    static void Register(EnumMetaBase* meta) { m_allEnumMetas.push_back(meta); }
public:
    EnumMetaBase(const EnumMetaBase&) = delete;
    EnumMetaBase& operator=(const EnumMetaBase&) = delete;

    static EnumMetaBase* FindByType(const std::type_info& typeInfo) {
        for (auto& meta : m_allEnumMetas) {
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

    List<EnumInfo>::const_iterator begin() const { return m_values.begin(); }
    List<EnumInfo>::const_iterator end() const { return m_values.end(); }
    List<const char*> GetValues() const {
        List<const char*> values;
        values.reserve(m_values.size());
        for (const auto& value : m_values) {
            values.push_back(value.name);
        }
        return values;
    }
    List<EnumInfo>::const_iterator GetValueInfo(StringView name) const {
        return std::find_if(m_values.begin(), m_values.end(),
            [&name](const EnumInfo& info) { return name == info.name; });
    }
};

template <typename BaseType, typename EnumT, typename InfoDetail>
class EnumMeta: public EnumMetaBase<BaseType, InfoDetail> {
public:
    using EnumInfo = typename EnumMetaBase<BaseType, InfoDetail>::EnumInfo;
    using Enum = typename EnumMetaBase<BaseType, InfoDetail>::Enum;

    EnumMeta() : EnumMetaBase<BaseType, InfoDetail>(&typeid(EnumT)) {}

    Enum AddValue(const char* name, InfoDetail detail = InfoDetail{}) {
        EnumInfo info{name, detail};
        m_values.push_back(info);
        return &m_values.back();
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
