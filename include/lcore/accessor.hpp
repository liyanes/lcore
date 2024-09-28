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
    inline virtual void Delete();
};

/// @brief Raw pointer accessor
/// @tparam T The type of the accessor
template <typename T>
class AccessorPtrImpl: public AccessorBase<T> {
    RawPtr<T> m_ptr;
public:
    inline AccessorPtrImpl(T* ptr);
    inline AccessorPtrImpl(RawPtr<T> ptr);
    inline const T Get() const override;
    inline void Set(const T& value) override;
    inline void Delete() override;

    inline RawPtr<T> GetPtr() const noexcept;
};

/// @brief Function accessor
/// @tparam FGet Get function type
/// @tparam FSet Set function type
/// @tparam FDelete Delete function type
template <typename FGet, typename FSet, typename FDelete>
requires IsCallable<FSet, ResultCallable<FGet>> && IsCallable<FDelete>
class AccessorFuncImpl: public AccessorBase<ResultCallable<FGet>> {
public:
    using RetType = ResultCallable<FGet>;
private:
    FGet m_get;
    FSet m_set;
    FDelete m_delete;
public:
    inline AccessorFuncImpl(const FGet& get, const FSet& set, const FDelete& del);
    inline AccessorFuncImpl(FGet&& get, FSet&& set, FDelete&& del);
    inline const RetType Get() const override;
    inline void Set(const RetType& value) override;
    inline void Delete() override;
};

/// @brief Accessor
/// @tparam T The type of the accessor
template <typename T>
class Accessor: Ptr<AccessorBase<T>> {
public:
    inline Accessor(Ptr<AccessorBase<T>> ptr);
    inline const T& Get() const;
    inline void Set(const T& value);
    inline void Delete();
};

template <typename T>
inline Accessor<T> MakeAccessor(T* ptr);

template <typename T>
inline Accessor<T> MakeAccessor(RawPtr<T> ptr);

template <typename FGet, typename FSet, typename FDelete>
inline Accessor<ResultCallable<FGet>> MakeAccessor(const FGet& get, const FSet& set, const FDelete& del);

template <typename FGet, typename FSet>
inline Accessor<ResultCallable<FGet>> MakeAccessor(const FGet& get, const FSet& set);

template <typename TType, typename FType>
requires IsDerivedFrom<TType, FType>
inline Accessor<Ptr<FType>> CreatePtrAccessor(Ptr<TType>* pptr) {
    return MakeAccessor([pptr]()->Ptr<FType>{return *pptr;}, [pptr](Ptr<FType> value){*pptr = std::dynamic_pointer_cast<TType>(value);});
}

LCORE_NAMESPACE_END

template <typename T>
inline void LCORE_NAMESPACE_NAME::AccessorBase<T>::Delete() {
    LCORE_NOTIMPLEMENTED();
}

template <typename T>
inline LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::AccessorPtrImpl(T* ptr): m_ptr(ptr) {}

template <typename T>
inline LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::AccessorPtrImpl(RawPtr<T> ptr): m_ptr(ptr) {}

template <typename T>
inline const T LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::Get() const {
    return *m_ptr;
}

template <typename T>
inline void LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::Set(const T& value) {
    *m_ptr = value;
}

template <typename T>
inline void LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::Delete() {
    delete m_ptr.Get();
}

template <typename T>
inline LCORE_NAMESPACE_NAME::RawPtr<T> LCORE_NAMESPACE_NAME::AccessorPtrImpl<T>::GetPtr() const noexcept {
    return m_ptr;
}

template <typename FGet, typename FSet, typename FDelete>
inline LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::AccessorFuncImpl(const FGet& get, const FSet& set, const FDelete& del): m_get(get), m_set(set), m_delete(del) {}

template <typename FGet, typename FSet, typename FDelete>
inline LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::AccessorFuncImpl(FGet&& get, FSet&& set, FDelete&& del): m_get(std::move(get)), m_set(std::move(set)), m_delete(std::move(del)) {}

template <typename FGet, typename FSet, typename FDelete>
inline const typename LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::RetType LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::Get() const {
    return m_get();
}

template <typename FGet, typename FSet, typename FDelete>
inline void LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::Set(const typename LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::RetType& value) {
    m_set(value);
}

template <typename FGet, typename FSet, typename FDelete>
inline void LCORE_NAMESPACE_NAME::AccessorFuncImpl<FGet, FSet, FDelete>::Delete() {
    m_delete();
}

template <typename T>
inline LCORE_NAMESPACE_NAME::Accessor<T>::Accessor(LCORE_NAMESPACE_NAME::Ptr<LCORE_NAMESPACE_NAME::AccessorBase<T>> ptr): Ptr<AccessorBase<T>>(ptr) {}

template <typename T>
inline const T& LCORE_NAMESPACE_NAME::Accessor<T>::Get() const {
    return Ptr<AccessorBase<T>>::operator->().Get();
}

template <typename T>
inline void LCORE_NAMESPACE_NAME::Accessor<T>::Set(const T& value) {
    Ptr<AccessorBase<T>>::operator->().Set(value);
}

template <typename T>
inline void LCORE_NAMESPACE_NAME::Accessor<T>::Delete() {
    Ptr<AccessorBase<T>>::operator->().Delete();
}

template <typename T>
inline LCORE_NAMESPACE_NAME::Accessor<T> LCORE_NAMESPACE_NAME::MakeAccessor(T* ptr) {
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename T>
inline LCORE_NAMESPACE_NAME::Accessor<T> LCORE_NAMESPACE_NAME::MakeAccessor(RawPtr<T> ptr) {
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename FGet, typename FSet, typename FDelete>
inline LCORE_NAMESPACE_NAME::Accessor<LCORE_NAMESPACE_NAME::ResultCallable<FGet>> LCORE_NAMESPACE_NAME::MakeAccessor(const FGet& get, const FSet& set, const FDelete& del) {
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, FDelete>(get, set, del));
}

template <typename FGet, typename FSet>
inline LCORE_NAMESPACE_NAME::Accessor<LCORE_NAMESPACE_NAME::ResultCallable<FGet>> LCORE_NAMESPACE_NAME::MakeAccessor(const FGet& get, const FSet& set) {
    auto del = [](){
        LCORE_NOTIMPLEMENTED();
    };
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, decltype(del)>(get, set, del));
}
