#pragma once
#include "base.hpp"
#include "ref.hpp"
#include "lcore/container.hpp"

LCORE_REFLECTION_NAMESPACE_BEGIN

enum class ClassAccess {
    Public,
    Protected,
    Private
};

class ReflectedField {
public:
    using Access = ClassAccess;
    
    TypeInfoPtr type;
    StringView name;
    bool is_const;
    bool is_volatile;
    bool is_mutable;
    Access access;

    int bitwidth = -1; // For bit-fields, -1 if not a bit-field
    
    void (*getter)(void* instance, void* out_value);
    void (*setter)(void* instance, const void* in_value);

    template <typename ClassType, typename FieldType>
    static ReflectedField Create(
        FieldType ClassType::* field, 
        StringView name, 
        Access access = Access::Public,
        bool is_mutable = false,
        int bitwidth = -1
    ) {
        ReflectedField result;
        result.type = &typeid(FieldType);
        result.name = name;
        result.is_const = std::is_const_v<FieldType>;
        result.is_volatile = std::is_volatile_v<FieldType>;
        result.is_mutable = is_mutable;
        result.access = access;
        result.bitwidth = bitwidth;

        result.getter = [](void* instance, void* out_value) {
            ClassType* obj = static_cast<ClassType*>(instance);
            FieldType* out = static_cast<FieldType*>(out_value);
            *out = obj->*field;
        };
        result.setter = [](void* instance, const void* in_value) {
            ClassType* obj = static_cast<ClassType*>(instance);
            const FieldType* in = static_cast<const FieldType*>(in_value);
            obj->*field = *in;
        };
        return result;
    }
};

class ReflectedMethod {
public:
    using Access = ClassAccess;

    TypeInfoPtr return_type;
    Span<const TypeInfoPtr> param_types;
    StringView name;
    Access access;
    bool is_const;
    bool is_static;
    bool is_virtual;
    bool is_pure_virtual;
    void (*invoker)(void* instance, void** args, void* out_return);

    template <typename Ret, typename ClassType, typename... Args>
    static ReflectedMethod Create(
        Ret(ClassType::* method)(Args...),
        StringView name,
        Access access = Access::Public,
        bool is_const = false,
        bool is_static = false,
        bool is_virtual = false,
        bool is_pure_virtual = false
    ) {
        ReflectedMethod result;
        result.return_type = &typeid(Ret);
        static TypeInfoPtr param_types_array[] = { &typeid(Args)... };
        result.param_types = Span<const TypeInfoPtr>(param_types_array, param_types_array + sizeof...(Args));
        result.name = name;
        result.access = access;
        result.is_const = is_const;
        result.is_static = is_static;
        result.is_virtual = is_virtual;
        result.is_pure_virtual = is_pure_virtual;
        result.invoker = [](void* instance, void** args, void* out_return) {
            ClassType* obj = static_cast<ClassType*>(instance);
            if constexpr (std::is_void_v<Ret>) {
                (obj->*method)(*static_cast<Args*>(args[0])...);
            } else {
                Ret* out = static_cast<Ret*>(out_return);
                *out = (obj->*method)(*static_cast<Args*>(args[0])...);
            }
        };
        return result;
    }
};

class ReflectedConstructor {
public:
    using Access = ClassAccess;
    enum class Type {
        Default,
        Copy,
        Move,
        Other
    };

    Span<const TypeInfoPtr> param_types;
    
    void (*invoker)(void* out_instance, void** args);

    template <typename ClassType, typename... Args>
    static ReflectedConstructor Create() {
        ReflectedConstructor result;
        static TypeInfoPtr param_types_array[] = { &typeid(Args)... };
        result.param_types = Span<const TypeInfoPtr>(param_types_array, param_types_array + sizeof...(Args));
        result.invoker = [](void* out_instance, void** args) {
            ClassType* obj = static_cast<ClassType*>(out_instance);
            new (obj) ClassType(*static_cast<Args*>(args[0])...);
        };
        return result;
    }
};

class ReflectedDestructor {
public:
    using Access = ClassAccess;
    void (*invoker)(void* instance);
    template <typename ClassType>
    static ReflectedDestructor Create() {
        ReflectedDestructor result;
        result.invoker = [](void* instance) {
            ClassType* obj = static_cast<ClassType*>(instance);
            obj->~ClassType();
        }
        return result;
    }
};

enum OperatorType {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    LogicalAnd,
    LogicalOr,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    LeftShift,
    RightShift,
    Negate,
    Not,
    Increment,
    Decrement,
    Index,
    Call
};

class ReflectedOperator {
public:
    using Access = ClassAccess;

    OperatorType op_type;
    TypeInfoPtr return_type;
    Span<const TypeInfoPtr> param_types;
    bool is_const;
    void (*invoker)(void* instance, void** args, void* out_return);

    template <typename Ret, typename ClassType, typename... Args>
    static ReflectedOperator Create(
        OperatorType op_type,
        Ret(ClassType::* method)(Args...),
        bool is_const = false,
        Access access = Access::Public
    ) {
        ReflectedOperator result;
        result.op_type = op_type;
        result.return_type = &typeid(Ret);
        static TypeInfoPtr param_types_array[] = { &typeid(Args)... };
        result.param_types = Span<const TypeInfoPtr>(param_types_array, param_types_array + sizeof...(Args));
        result.is_const = is_const;
        result.invoker = [](void* instance, void** args, void* out_return) {
            ClassType* obj = static_cast<ClassType*>(instance);
            if constexpr (std::is_void_v<Ret>) {
                (obj->*method)(*static_cast<Args*>(args[0])...);
            } else {
                Ret* out = static_cast<Ret*>(out_return);
                *out = (obj->*method)(*static_cast<Args*>(args[0])...);
            }
        };
        return result;
    }
};

class ReflectedStaticField {
public:
    TypeInfoPtr type;
    StringView name;
    bool is_const;
    bool is_volatile;
    int bitwidth = -1; // For bit-fields, -1 if not a bit-field
    void (*getter)(void* out_value);
    void (*setter)(const void* in_value);

    template <typename FieldType>
    static ReflectedStaticField Create(
        FieldType* field,
        StringView name,
        bool is_const = false,
        bool is_volatile = false,
        int bitwidth = -1
    ) {
        ReflectedStaticField result;
        result.type = &typeid(FieldType);
        result.name = name;
        result.is_const = is_const;
        result.is_volatile = is_volatile;
        result.bitwidth = bitwidth;
        result.getter = [](void* out_value) {
            FieldType* out = static_cast<FieldType*>(out_value);
            *out = *field;
        };
        result.setter = [](const void* in_value) {
            const FieldType* in = static_cast<const FieldType*>(in_value);
            *field = *in;
        };
        return result;
    }
};

class ReflectedStaticMethod {
public:
    TypeInfoPtr return_type;
    Span<const TypeInfoPtr> param_types;
    StringView name;
    void (*invoker)(void** args, void* out_return);

    template <typename Ret, typename... Args>
    static ReflectedStaticMethod Create(
        Ret(*method)(Args...),
        StringView name
    ) {
        ReflectedStaticMethod result;
        result.return_type = &typeid(Ret);
        static TypeInfoPtr param_types_array[] = { &typeid(Args)... };
        result.param_types = Span<const TypeInfoPtr>(param_types_array, param_types_array + sizeof...(Args));
        result.name = name;
        result.invoker = [](void** args, void* out_return) {
            if constexpr (std::is_void_v<Ret>) {
                (*method)(*static_cast<Args*>(args[0])...);
            } else {
                Ret* out = static_cast<Ret*>(out_return);
                *out = (*method)(*static_cast<Args*>(args[0])...);
            }
        };
        return result;
    }
};

// class ReflectedStaticOperator {
// public:
//     OperatorType op_type;
//     TypeInfoPtr return_type;
//     Span<const TypeInfoPtr> param_types;
//     void (*invoker)(void** args, void* out_return);

//     template <typename Ret, typename... Args>
//     static ReflectedStaticOperator Create(
//         OperatorType op_type,
//         Ret(*method)(Args...)
//     ) {
//         ReflectedStaticOperator result;
//         result.op_type = op_type;
//         result.return_type = &typeid(Ret);
//         static TypeInfoPtr param_types_array[] = { &typeid(Args)... };
//         result.param_types = Span<const TypeInfoPtr>(param_types_array, param_types_array + sizeof...(Args));
//         result.invoker = [](void** args, void* out_return) {
//             if constexpr (std::is_void_v<Ret>) {
//                 (*method)(*static_cast<Args*>(args[0])...);
//             } else {
//                 Ret* out = static_cast<Ret*>(out_return);
//                 *out = (*method)(*static_cast<Args*>(args[0])...);
//             }
//         };
//         return result;
//     }
// };

class ClassReflection: public ReflectionBase {
public:
    using ReflectionBase::ReflectionBase;

    Span<const ReflectedField> fields;
    Span<const ReflectedMethod> methods;
    Span<const ReflectedConstructor> constructors;
    RawPtr<const ReflectedDestructor> destructor; // Only one destructor per class
    Span<const ReflectedOperator> operators;
    Span<const ReflectedStaticField> static_fields;
    Span<const ReflectedStaticMethod> static_methods;
    // Span<const ReflectedStaticOperator> static_operators;

    TypeInfoRef GetReflectionType() const override {
        return typeid(ClassReflection);
    }
    String Serialize(const void* instance) const override {
        // Simple serialization: just return the class name
        return String("Instance of ") + StringView(type->name());
    }
};

LCORE_REFLECTION_NAMESPACE_END
