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

#include "../config.h"
#include "../pointer.hpp"
#include "../container.hpp"
#include "../exception.hpp"
#include "../assert.hpp"
#include "typeid.hpp"
#include <map>
#include <typeinfo>

LCORE_NAMESAPCE_BEGIN

class Object;

class Aggregator {
private:
    std::map<TypeId,Ptr<Object>> objects;
public:
    /// @brief The shared reference count, when count is 0, the aggregator will be destroyed (by Object)
    /// Initialize to 1
    size_t shared_ref_count = 1;

    Aggregator(){};
    ~Aggregator(){};

    template <typename T>
    requires IsDerivedFrom<T, Object>
    void AddObject(Ptr<T> obj){
        LCORE_ASSERT(obj != nullptr, "Object is nullptr");

        shared_ref_count += obj->aggregator->shared_ref_count;
        for (auto [tid, obj]: obj->aggregator->objects){
            auto [it, success] = objects.insert(std::make_pair<TypeId, Ptr<Object>>(T::GetTypeId(), obj));
            if (!success){
                LCORE_FATAL("Object with same type already exists");
            }
        }
        // Free the aggregator, ignore the shared_ref_count varible
        obj->aggregator = this;
    }

    template <typename T>
    requires IsDerivedFrom<T, Object>
    void RemoveObject(){
        auto it = objects.find(T::GetTypeId());
        if (it != objects.end()){
            objects.erase(it);
        }
    }

    template <typename T>
    requires IsDerivedFrom<T, Object>
    Ptr<T> GetObject(){
        auto it = objects.find(T::GetTypeId());
        if (it != objects.end()){
            return it->second.Cast<T>();
        }
        return nullptr;
    }
};

/// @brief Base object class for extending
/// Implement this class will allow the object to be aggregated
/// You need to implement the GetTypeId() function to get the type id of the object, and the GetInstanceTypeId() function to get the instance type id of the object
class Object: public AbstractClass {
    friend class Aggregator;
private:
    RawPtr<Aggregator> aggregator;
public:
    Object(): aggregator(new Aggregator()){};
    Object(const Object& obj): aggregator(obj.aggregator){};
    Object(Object&& obj): aggregator(std::move(obj.aggregator)){};
    ~Object(){};
    Object& operator=(const Object& obj){
        aggregator = obj.aggregator;
        return *this;
    }
    Object& operator=(Object&& obj){
        aggregator = std::move(obj.aggregator);
        return *this;
    }

    static TypeId GetTypeId(){
        const TypeInfo type{
            .name = "Object",
            .parent = nullptr,
            .constructor = []{return MakePtr<Object>();}
        };
        return TypeId(&type);
    }

    virtual TypeId GetInstanceTypeId() const {
        return GetTypeId();
    }

    template <typename T>
    requires IsDerivedFrom<T, Object>
    void Aggregate(Ptr<T> obj){
        aggregator->AddObject<T>(obj);
    }

    template <typename T>
    requires IsDerivedFrom<T, Object>
    Ptr<T> GetObject(){
        return aggregator->GetObject<T>();
    }
};

LCORE_NAMESAPCE_END
