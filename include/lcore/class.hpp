#pragma once
#include "base.hpp"

LCORE_NAMESPACE_BEGIN

/// @brief Singleton class
/// @tparam BaseClass The class to be created as a singleton
/// @example class TestClass: public Singleton<TestClass> {];
template <typename BaseClass>
class Singleton{
public:
    inline static BaseClass& Get() {
        /// @brief Instance of the singleton
        static BaseClass instance;
        return instance;
    };
protected:
    inline ~Singleton() = default;
    inline Singleton() = default;
public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

/// @brief Static class
class StaticClass {
    StaticClass() = delete;
    StaticClass(const StaticClass&) = delete;
    StaticClass(StaticClass&&) = delete;
    StaticClass& operator=(const StaticClass&) = delete;
    StaticClass& operator=(StaticClass&&) = delete;
};

/// @brief Mark a class as abstract
class AbstractClass {
public:
    virtual ~AbstractClass() = default;
};

/// @brief Just identify the class as an interface
/// User code should implement the interface themselves, this class is just a mark
class Interface {
};

LCORE_NAMESPACE_END
