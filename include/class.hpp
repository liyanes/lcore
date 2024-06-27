#pragma once
#include "base.hpp"

LCORE_NAMESAPCE_BEGIN

/// @brief Singleton class
/// @tparam BaseClass The class to be created as a singleton
/// @example class TestClass: public Singleton<TestClass> {];
template <typename BaseClass>
class Singleton{
public:
    static BaseClass* Get() {
        /// @brief Instance of the singleton
        static BaseClass* instance;
        if (instance == nullptr) instance = new BaseClass();
        return instance;
    };
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

LCORE_NAMESAPCE_END
