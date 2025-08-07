#pragma once
#include "base.hpp"
#include "pointer.hpp"
#include "exception.hpp"
#include "class.hpp"
#include <functional>

LCORE_NAMESPACE_BEGIN

/// @brief Accessor base class
/// @tparam T The type of the accessor
template <typename T>
class AccessorBase: AbstractClass {
    /// @brief Get the value
    /// @return The value
    virtual const T Get() const = 0;
    /// @brief Set the value
    /// @param value The value to be set
    virtual void Set(const T& value) = 0;
    /// @brief Delete the value
    inline virtual void Delete() {
        LCORE_NOTIMPLEMENTED();
    }
};

/// @brief Raw pointer accessor
/// @tparam T The type of the accessor
template <typename T>
class AccessorPtrImpl: public AccessorBase<T> {
    RawPtr<T> m_ptr;
public:
    inline AccessorPtrImpl(T* ptr): m_ptr(ptr) {}
    inline AccessorPtrImpl(RawPtr<T> ptr): m_ptr(ptr) {}
    inline const T Get() const override {return *m_ptr;}
    inline void Set(const T& value) override {*m_ptr = value;}
    inline void Delete() override {delete m_ptr.Get();}

    inline RawPtr<T> GetPtr() const noexcept {return m_ptr;}
};

template <typename FGet, typename FSet, typename FDelete>
concept IsAccessorFunctuple = requires(FGet _get, FSet _set, FDelete _delete){
    // {_get()} -> Reference;s
    _set(_get());
    _delete();
};

/// @brief Function accessor
/// @tparam FGet Get function type
/// @tparam FSet Set function type
/// @tparam FDelete Delete function type
template <typename FGet, typename FSet, typename FDelete>
requires IsAccessorFunctuple<FGet, FSet, FDelete>
class AccessorFuncImpl: public AccessorBase<ResultCallable<FGet>> {
public:
    using RetType = ResultCallable<FGet>;
private:
    FGet m_get;
    FSet m_set;
    FDelete m_delete;
public:
    inline AccessorFuncImpl(const FGet& get, const FSet& set, const FDelete& del):
        m_get(get), m_set(set), m_delete(del){}
    inline AccessorFuncImpl(FGet&& get, FSet&& set, FDelete&& del):
        m_get(std::move(get)), m_set(std::move(set)), m_delete(std::move(del)){}
    inline const RetType Get() const override{return m_get();}
    inline void Set(const RetType& value) override{m_set(value);}
    inline void Delete() override{m_delete();}
};

/// @brief Accessor
/// @tparam T The type of the accessor
template <typename T>
class Accessor: Ptr<AccessorBase<T>> {
public:
    inline Accessor(Ptr<AccessorBase<T>> ptr): Ptr<AccessorBase<T>>(ptr) {}
    inline const T& Get() const{return Ptr<AccessorBase<T>>::operator->().Get();}
    inline void Set(const T& value){Ptr<AccessorBase<T>>::operator->().Set(value);};
    inline void Delete(){Ptr<AccessorBase<T>>::operator->().Delete();};
};

template <typename T>
inline Accessor<T> MakeAccessor(T* ptr){
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename T>
inline Accessor<T> MakeAccessor(RawPtr<T> ptr){
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename FGet, typename FSet, typename FDelete>
inline Accessor<ResultCallable<FGet>> MakeAccessor(const FGet& get, const FSet& set, const FDelete& del){
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, FDelete>(get, set, del));
}

template <typename FGet, typename FSet>
inline Accessor<ResultCallable<FGet>> MakeAccessor(const FGet& get, const FSet& set){
    auto del = [](){
        LCORE_NOTIMPLEMENTED();
    };
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, decltype(del)>(get, set, del));
}

template <typename TType, typename FType>
requires DerivedFrom<TType, FType>
inline Accessor<Ptr<FType>> CreatePtrAccessor(Ptr<TType>* pptr) {
    return MakeAccessor([pptr]()->Ptr<FType>{return *pptr;}, [pptr](Ptr<FType> value){*pptr = std::dynamic_pointer_cast<TType>(value);});
}

LCORE_NAMESPACE_END
