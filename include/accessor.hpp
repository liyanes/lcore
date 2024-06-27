#pragma once
#include "base.hpp"
#include "pointer.hpp"
#include "exception.hpp"
#include "class.hpp"
#include <functional>

LCORE_NAMESAPCE_BEGIN

/// @brief Accessor base class
/// @tparam T The type of the accessor
template <typename T>
class AccessorBase: AbstractClass {
    /// @brief Get the value
    /// @return The value
    virtual const T& Get() const = 0;
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
    inline const T& Get() const override;
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
    using T = ResultCallable<FGet>;
private:
    FGet m_get;
    FSet m_set;
    FDelete m_delete;
public:
    inline AccessorFuncImpl(const FGet& get, const FSet& set, const FDelete& del);
    inline AccessorFuncImpl(FGet&& get, FSet&& set, FDelete&& del);
    inline const T& Get() const override;
    inline void Set(const T& value) override;
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

LCORE_NAMESAPCE_END



template <typename T>
inline void lcore::AccessorBase<T>::Delete() {
    throw NotImplementedError(__func__, __FILE__, __LINE__);
}

template <typename T>
inline lcore::AccessorPtrImpl<T>::AccessorPtrImpl(T* ptr): m_ptr(ptr) {}

template <typename T>
inline lcore::AccessorPtrImpl<T>::AccessorPtrImpl(RawPtr<T> ptr): m_ptr(ptr) {}

template <typename T>
inline const T& lcore::AccessorPtrImpl<T>::Get() const {
    return *m_ptr;
}

template <typename T>
inline void lcore::AccessorPtrImpl<T>::Set(const T& value) {
    *m_ptr = value;
}

template <typename T>
inline void lcore::AccessorPtrImpl<T>::Delete() {
    delete m_ptr.Get();
}

template <typename T>
inline lcore::RawPtr<T> lcore::AccessorPtrImpl<T>::GetPtr() const noexcept {
    return m_ptr;
}

template <typename FGet, typename FSet, typename FDelete>
inline lcore::AccessorFuncImpl<FGet, FSet, FDelete>::AccessorFuncImpl(const FGet& get, const FSet& set, const FDelete& del): m_get(get), m_set(set), m_delete(del) {}

template <typename FGet, typename FSet, typename FDelete>
inline lcore::AccessorFuncImpl<FGet, FSet, FDelete>::AccessorFuncImpl(FGet&& get, FSet&& set, FDelete&& del): m_get(std::move(get)), m_set(std::move(set)), m_delete(std::move(del)) {}

template <typename FGet, typename FSet, typename FDelete>
inline const typename lcore::AccessorFuncImpl<FGet, FSet, FDelete>::T& lcore::AccessorFuncImpl<FGet, FSet, FDelete>::Get() const {
    return m_get();
}

template <typename FGet, typename FSet, typename FDelete>
inline void lcore::AccessorFuncImpl<FGet, FSet, FDelete>::Set(const typename lcore::AccessorFuncImpl<FGet, FSet, FDelete>::T& value) {
    m_set(value);
}

template <typename FGet, typename FSet, typename FDelete>
inline void lcore::AccessorFuncImpl<FGet, FSet, FDelete>::Delete() {
    m_delete();
}

template <typename T>
inline lcore::Accessor<T>::Accessor(lcore::Ptr<lcore::AccessorBase<T>> ptr): Ptr(ptr) {}

template <typename T>
inline const T& lcore::Accessor<T>::Get() const {
    return Ptr::Get()->Get();
}

template <typename T>
inline void lcore::Accessor<T>::Set(const T& value) {
    Ptr::Get()->Set(value);
}

template <typename T>
inline void lcore::Accessor<T>::Delete() {
    Ptr::Get()->Delete();
}

template <typename T>
inline lcore::Accessor<T> lcore::MakeAccessor(T* ptr) {
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename T>
inline lcore::Accessor<T> lcore::MakeAccessor(RawPtr<T> ptr) {
    return Ptr<AccessorBase<T>>(new AccessorPtrImpl<T>(ptr));
}

template <typename FGet, typename FSet, typename FDelete>
inline lcore::Accessor<lcore::ResultCallable<FGet>> lcore::MakeAccessor(const FGet& get, const FSet& set, const FDelete& del) {
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, FDelete>(get, set, del));
}

template <typename FGet, typename FSet>
inline lcore::Accessor<lcore::ResultCallable<FGet>> lcore::MakeAccessor(const FGet& get, const FSet& set) {
    return Ptr<AccessorBase<ResultCallable<FGet>>>(new AccessorFuncImpl<FGet, FSet, auto>(get, set, [](){
        LCORE_NOTIMPLEMENTED();
    }));
}
