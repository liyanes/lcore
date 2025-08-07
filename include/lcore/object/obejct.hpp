/**
 * @file obejct.hpp
 * @author liyanes@outlook.com
 * @brief Base object class, implements the basic object operations and aggregation
 * @version 0.1
 * @date 2024-09-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include "base.hpp"
#include "../pointer.hpp"
#include "../container.hpp"
#include "../exception.hpp"
#include "../assert.hpp"
#include "typeid.hpp"
#include <map>
#include <typeinfo>

LCORE_OBJ_NAMESPACE_BEGIN

class Object;

class ObjectDeleter {
public:
    inline static void Delete(Object* obj);
};

class Aggregator {
private:
    friend class ObjectDeleter;
    struct ExposedObjectPtr {
        RawPtr<Object> obj;
        size_t ptr_ref_count = 0;
    };
    std::map<TypeId, ExposedObjectPtr> objects;
public:
    /// @brief The shared reference count, when count is 0, the aggregator will be destroyed (by Object)
    size_t shared_ref_count;

    template <typename T>
    requires DerivedFrom<T, Object>
    Aggregator(RawPtr<T> createdObject): shared_ref_count(1) {
        if (!objects.insert({T::GetTypeId(), {createdObject, 1}}).second) 
            LCORE_FATAL("Failed to create aggregator");
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    Aggregator(RawPtr<T> createdObject, size_t initial_count): shared_ref_count(initial_count) {
        if (!objects.insert({T::GetTypeId(), {createdObject, initial_count}}).second) 
            LCORE_FATAL("Failed to create aggregator");
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    void AddObject(RawPtr<T> obj){
        LCORE_ASSERT(obj != nullptr, "Object is nullptr");

        shared_ref_count += obj->aggregator->shared_ref_count;
        auto objagg = obj->aggregator;

        for (auto [tid, epobj]: obj->aggregator->objects){
            epobj.obj->aggregator = this;
            auto [it, success] = objects.insert({T::GetTypeId(), epobj});
            if (!success){
                LCORE_FATAL("Object with same type already exists");
            }
        }
        // Free the aggregator, ignore the shared_ref_count varible
        objagg.Delete();
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    Ptr<T> ReleaseObject(){
        auto it = objects.find(T::GetTypeId());
        if (it != objects.end()){    
            if (objects.size() == 1){
                // Only one object left, return the object itself
                return Ptr<T>((T*)&*it->second.obj, ObjectDeleter::Delete);
            }
            auto ptr_ref_count = it->second.ptr_ref_count;
            shared_ref_count -= ptr_ref_count;
            auto obj = it->second.obj;
            objects.erase(it);
            if (shared_ref_count == 0){
                // After release, the shared_ref_count is 0, delete the aggregator and all objects
                ObjectDeleter::Delete(objects.begin()->second.obj.Get());
            }
            obj->aggregator = new Aggregator(obj, ptr_ref_count + 1);   // Due to the return value, the ptr_ref_count should be increased by 1
            return Ptr<T>((T*)&*obj, ObjectDeleter::Delete);
        }
        return nullptr;
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    Ptr<T> GetObject(){
        auto it = objects.find(T::GetTypeId());
        if (it != objects.end()){
            it->second.ptr_ref_count++;
            shared_ref_count++;
            return Ptr<T>((T*)&*it->second.obj, ObjectDeleter::Delete);
        }
        return nullptr;
    }
};

/// @brief Base object class for extending
/// Implement this class will allow the object to be aggregated
/// You need to implement the GetTypeId() function to get the type id of the object, and the GetInstanceTypeId() function to get the instance type id of the object
class Object: public AbstractClass {
    friend class Aggregator;
    friend class ObjectDeleter;
private:
    RawPtr<Aggregator> aggregator = nullptr;
protected:
    /// @brief Construct a new Object object (for derived class)
    /// @note User code should not call this function
    inline Object() {};
public:
    inline Object(const Object& obj): aggregator(obj.aggregator){      // What is this?
        if (aggregator != nullptr)
            aggregator->shared_ref_count++;
    };

    inline Object(Object&& obj): aggregator(std::move(obj.aggregator)){
        obj.aggregator = nullptr;
    };

    // virtual ~Object(){}

    Object& operator=(const Object& obj){
        aggregator = obj.aggregator;
        return *this;
    }
    Object& operator=(Object&& obj){
        aggregator = std::move(obj.aggregator);
        return *this;
    }

    template <typename T, typename ...Args>
    requires DerivedFrom<T, Object>
    inline static Ptr<T> New(Args ...args){
        Ptr<T> obj = Ptr<T>(new T(std::forward(args)...), ObjectDeleter::Delete);
        obj->aggregator = new Aggregator(RawPtr<T>(obj.Get()));
        return obj;
    }

    inline static TypeId GetTypeId(){
        const TypeInfo type{
            .name = "Object",
            .parent = nullptr,
            .constructor = []{return Object::New<Object>();}
        };
        return TypeId(&type);
    }

    virtual TypeId GetInstanceTypeId() const {
        return GetTypeId();
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    inline void Aggregate(Ptr<T> obj){
        aggregator->AddObject<T>(obj.Get());
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    inline Ptr<T> GetObject(){
        return aggregator->GetObject<T>();
    }

    template <typename T>
    requires DerivedFrom<T, Object>
    inline Ptr<T> ReleaseObject(){
        return aggregator->ReleaseObject<T>();
    }
};

inline void ObjectDeleter::Delete(Object* obj){
    if (!obj->aggregator) {
        delete obj;
        return;
    }
    for (auto& [tid, epobj]: obj->aggregator->objects){
        if (epobj.obj == obj){
            epobj.ptr_ref_count--;
            break;
        }
    }
    obj->aggregator->shared_ref_count--;
    if (obj->aggregator->shared_ref_count == 0){
        auto objagg = obj->aggregator;
        for (auto& [tid, epobj]: obj->aggregator->objects){
            epobj.obj.Delete();
        }
        objagg.Delete();
    }
}

/// @brief Create a new object
/// @tparam T The type of the object
/// @tparam ...Args The arguments to pass to the constructor
/// @param ...args The arguments to pass to the constructor
/// @return Ptr<T> The created object
template <typename T, typename ...Args>
requires DerivedFrom<T, Object>
Ptr<T> NewObject(Args ...args){
    return Object::New<T>(std::forward(args)...);
}

LCORE_OBJ_NAMESPACE_END
