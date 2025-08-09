#pragma once
#include "base.hpp"
#include "traits.hpp"
#include <atomic>
#include <memory>

#ifdef LCORE_DEBUG
#include "assert.hpp"
#define _LCORE_CHECK_PTR_NOTZERO(ptr) LCORE_ASSERT(ptr, "Try to dereference a null pointer")
#else
#define _LCORE_CHECK_PTR_NOTZERO(ptr) do {} while (0)
#endif

LCORE_NAMESPACE_BEGIN

// Forward declarations
template <typename T>
class RawPtr;

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

/// @brief Raw pointer
/// @tparam T The type of the pointer
template <typename T>
class RawPtr {
    template <typename U>
    friend class RawPtr;
protected:
    T* ptr = nullptr;
public:
    using Type = T;
    using Pointer = Type*;
    using ConstPointer = const Type*;

    inline constexpr RawPtr() = default;
    inline constexpr RawPtr(T* ptr): ptr(ptr) {}
    inline constexpr RawPtr(std::nullptr_t): ptr(nullptr) {}
    inline constexpr RawPtr(const RawPtr<T>& ptr): ptr(ptr.ptr) {}
    inline constexpr RawPtr(RawPtr<T>&& ptr): ptr(ptr.ptr) {ptr.ptr = nullptr;}
    template <typename U>
    requires (DerivedFrom<U, T> || Same<T, void>)
    inline constexpr RawPtr(const RawPtr<U>& ptr): ptr(ptr.ptr) {}

    inline constexpr RawPtr<T>& operator=(const RawPtr<T>& ptr) noexcept {this->ptr = ptr.ptr; return *this;}
    inline constexpr RawPtr<T>& operator=(RawPtr<T>&& ptr) noexcept {this->ptr = ptr.ptr; ptr.ptr = nullptr; return *this;}
    template <typename U>
    requires (DerivedFrom<U, T> || Same<T, void>)
    inline constexpr RawPtr<T>& operator=(const RawPtr<U>& ptr) noexcept { this->ptr = ptr.ptr; return *this; }
    
    // inline constexpr RawPtr<T>& operator=(T* ptr) noexcept {this->ptr = ptr; return *this;}
    // template <typename U>
    // requires (DerivedFrom<U, T> || Same<T, void>)
    // inline constexpr RawPtr<T>& operator=(U* ptr) noexcept { this->ptr = static_cast<T*>(ptr); return *this; }
    
    inline constexpr RawPtr<T>& operator=(std::nullptr_t) noexcept {this->ptr = nullptr; return *this;}
    
    inline constexpr T* operator->() const noexcept {
        _LCORE_CHECK_PTR_NOTZERO(ptr);
        return ptr;
    }
    inline constexpr auto operator*() const noexcept requires (!Void<T>) {
        _LCORE_CHECK_PTR_NOTZERO(ptr);
        return *ptr;
    }
    // inline constexpr void operator*() const requires Void<T> {
    //     static_assert(!Void<T>, "Cannot dereference a void pointer");
    // }

    inline constexpr operator bool() const noexcept {return ptr != nullptr;}
    inline constexpr auto operator<=>(const RawPtr<T>& other) const noexcept {
        return ptr <=> other.ptr;
    }
    inline constexpr auto operator==(std::nullptr_t) const noexcept { return ptr == nullptr; }
    inline constexpr auto operator!=(std::nullptr_t) const noexcept { return ptr != nullptr; }
    template <typename U>
    inline constexpr auto operator==(const RawPtr<U>& other) const noexcept { return ptr == other.ptr; }
    template <typename U>
    inline constexpr auto operator!=(const RawPtr<U>& other) const noexcept { return ptr != other.ptr; }
    template <typename U>
    inline constexpr auto operator==(const U* other) const noexcept { return ptr == other; }
    template <typename U>
    inline constexpr auto operator!=(const U* other) const noexcept { return ptr != other; }

    inline constexpr bool IsConst() const noexcept {return std::is_const_v<T>;}
    inline constexpr T* Get() const noexcept {return ptr;}
    inline constexpr void Delete() noexcept {
        delete ptr;
        ptr = nullptr;
    }
    inline constexpr void Reset() noexcept {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }
    inline constexpr void Swap(RawPtr<T>& other) noexcept {
        std::swap(ptr, other.ptr);
    }

    template <typename U>
    requires DerivedFrom<U, T> || DerivedFrom<T, U> || OneOf<void, T, U>
    inline constexpr RawPtr<U> Cast() const noexcept {
        return static_cast<U*>(ptr);
    }

    template <typename U>
    inline constexpr RawPtr<U> DynamicCast() const noexcept {
        return dynamic_cast<U*>(ptr);
    }

    template <typename U>
    requires Same<RemoveCV<T>, RemoveCV<U>>
    inline constexpr RawPtr<U> ConstCast() const noexcept {
        return const_cast<U*>(ptr);
    }

    template <typename U>
    inline constexpr RawPtr<U> ReinterpretCast() const noexcept {
        return reinterpret_cast<U*>(ptr);
    }
};

namespace _detail {

// Control block for SharedPtr & WeakPtr
template <template <typename> typename AtomicType = std::atomic>
class ControlBlockBase {
public:
    void* ptr;    // ptr will not updated until the control block is destroyed, we don't need to use atomic for it
    AtomicType<size_t> shared_count = 1; // Start with 1 for the initial shared_ptr
    AtomicType<size_t> weak_count = 0;

    ControlBlockBase(void* p): ptr(p) {}
    template <typename T>
    ControlBlockBase(RawPtr<T> p): ptr(p.Get()) {}

    /// @brief Destroy the object (Do not deallocate the memory!!!)
    virtual void Destory() = 0;
    /// @brief Deallocate the memory of this control block
    virtual void Deallocate() = 0;
    virtual ~ControlBlockBase() = default;

    void Ref() noexcept { ++shared_count; }
    bool Unref() {
        if (--shared_count == 0) {
            /**
             * Why weak_count is cached here?
             * When Destory() is called, the target object may own weak references to this control block.
             * WeakUnref() will be called and weak_count will be decremented.
             * if weak_count reaches 0, this control block will be deallocated by WeakUnref().
             * When Destory() is returned, this control block is deallocated and the weak_count is no longer valid.
             * So we cache the weak_count before calling Destory() to ensure that we can safely check if the control block is deallocated.
             */
            size_t weak_count = this->weak_count;
            Destory();
            if (weak_count == 0) {
                Deallocate();
                return true; // Indicates that the control block is deallocated
            }
        }
        return false; // Indicates that the control block is still alive
    }
    void WeakRef() noexcept { ++weak_count; }
    bool WeakUnref() {
        if (--weak_count == 0 && shared_count == 0) {
            Deallocate();
            return true; // Indicates that the control block is deallocated
        }
        return false; // Indicates that the control block is still alive
    }
};

template <typename T, template <typename> typename AtomicType = std::atomic>
class ControlBlock : public ControlBlockBase<AtomicType> {
public:
    using Pointer = RawPtr<T>;
    
    ControlBlock(Pointer p): ControlBlockBase<AtomicType>(p) {};
    
    void Destory() override {
        delete static_cast<T*>(this->ptr);
    }

    void Deallocate() override {
        delete this; // Deallocate the control block itself
    }
};

template <typename T, typename Deleter, template <typename> typename AtomicType = std::atomic>
class ControlBlockDeleter : public ControlBlockBase<AtomicType> {
public:
    using Pointer = RawPtr<T>;
    
    Deleter deleter;

    ControlBlockDeleter(Pointer p, Deleter d): ControlBlockBase<AtomicType>(p), deleter(std::move(d)) {};

    void Destory() override {
        deleter(static_cast<T*>(this->ptr));
    }

    void Deallocate() override {
        delete this; // Deallocate the control block itself
    }
};

template <typename T, typename Deleter, typename Allocator, template <typename> typename AtomicType = std::atomic>
class ControlBlockDeleterAllocator : public ControlBlockBase<AtomicType> {
public:
    using Pointer = RawPtr<T>;

    Deleter deleter;
    Allocator allocator;

    ControlBlockDeleterAllocator(Pointer p, Deleter d, Allocator a):
        ControlBlockBase<AtomicType>(p), deleter(std::move(d)), allocator(std::move(a)) {};

    void Destory() override {
        deleter(this->ptr);
    }

    void Deallocate() override {
        // using AllocTraits = std::allocator_traits<Allocator>;
        auto alloc = allocator;
        Allocator::destroy(alloc, this);
        Allocator::deallocate(alloc, this, 1);
    }
};

}

/// @brief Shared pointer
/// @tparam T The type of the pointer
template <typename T>
class SharedPtr {
    template <typename U>
    friend class SharedPtr;
    template <typename U>
    friend class WeakPtr;
protected:
    RawPtr<T> m_tptr;
    RawPtr<_detail::ControlBlockBase<>> m_cb = nullptr;
    inline SharedPtr(RawPtr<T> tptr, RawPtr<_detail::ControlBlockBase<>> cb): m_tptr(tptr), m_cb(cb) { if (cb) m_cb->Ref(); }
public:
    // Types
    using Type = T;
    // factory methods
    inline SharedPtr() = default;
    inline SharedPtr(std::nullptr_t): m_tptr(nullptr), m_cb(nullptr) {}
    inline SharedPtr(RawPtr<T> ptr): m_tptr(ptr), m_cb(new _detail::ControlBlock<T>(ptr)) {}
    template <typename Deleter>
    inline SharedPtr(RawPtr<T> ptr, Deleter deleter): m_tptr(ptr), m_cb(new _detail::ControlBlockDeleter<T, Deleter>(ptr, std::move(deleter))) {}
    template <typename Deleter, typename Allocator>
    inline SharedPtr(RawPtr<T> ptr, Deleter deleter, Allocator allocator)
        : m_tptr(ptr), m_cb(new _detail::ControlBlockDeleterAllocator<T, Deleter, Allocator>(ptr, std::move(deleter), std::move(allocator))) {}
    inline SharedPtr(const SharedPtr<T>& other) noexcept: m_tptr(other.m_tptr), m_cb(other.m_cb) {
        if (m_cb) m_cb->Ref();
    }
    inline SharedPtr(SharedPtr<T>&& other) noexcept: m_tptr(std::move(other.m_tptr)), m_cb(std::move(other.m_cb)) {
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
    }
    template <typename U>
    requires DerivedFrom<U, T> || Same<T, void>
    inline SharedPtr(const SharedPtr<U>& other) noexcept: m_tptr(other.m_tptr.template Cast<T>()), m_cb(other.m_cb) {
        if (m_cb) m_cb->Ref();
    }
    template <typename U>
    requires DerivedFrom<U, T> || Same<T, void>
    inline SharedPtr(SharedPtr<U>&& other) noexcept: m_tptr(other.m_tptr.template Cast<T>()), m_cb(other.m_cb) {
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
    }
    inline ~SharedPtr() {
        if (m_cb) {
            m_cb->Unref();
            m_cb = nullptr; // Prevent dangling pointer
            m_tptr = nullptr;
        }
    }

    // operators
    inline SharedPtr<T>& operator=(const SharedPtr<T>& other) noexcept {
        if (this != &other) {
            if (m_cb) m_cb->Unref();
            m_tptr = other.m_tptr;
            m_cb = other.m_cb;
            if (m_cb) m_cb->Ref();
        }
        return *this;
    }
    inline SharedPtr<T>& operator=(SharedPtr<T>&& other) noexcept {
        if (this != &other) {
            if (m_cb) m_cb->Unref();
            m_tptr = std::move(other.m_tptr);
            m_cb = std::move(other.m_cb);
            other.m_tptr = nullptr;
            other.m_cb = nullptr;
        }
        return *this;
    }
    template <typename U>
    requires (DerivedFrom<U, T> && !Same<U, T>) || Same<T, void>
    inline SharedPtr<T>& operator=(const SharedPtr<U>& other) noexcept {
        if (m_cb) m_cb->Unref();
        m_tptr = other.m_tptr.template Cast<T>();
        m_cb = other.m_cb;
        if (m_cb) m_cb->Ref();
        return *this;
    }
    template <typename U>
    requires (DerivedFrom<U, T> && !Same<U, T>) || Same<T, void>
    inline SharedPtr<T>& operator=(SharedPtr<U>&& other) noexcept {
        if (m_cb) m_cb->Unref();
        m_tptr = other.m_tptr.template Cast<T>();
        m_cb = std::move(other.m_cb);
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
        return *this;
    }

    inline T* operator->() const noexcept {
        _LCORE_CHECK_PTR_NOTZERO(m_tptr.Get());
        return m_tptr.operator->();
    }
    inline auto operator*() const noexcept requires (!Void<T>) {
        _LCORE_CHECK_PTR_NOTZERO(m_tptr.Get());
        return m_tptr.operator*();
    }

    inline operator bool() const noexcept { return m_tptr != nullptr; }
    inline int operator<=>(const SharedPtr<T>& other) const noexcept {
        return m_tptr <=> other.m_tptr;
    }

    // Interface methods

    inline constexpr bool IsConst() const noexcept { return std::is_const_v<T>; }
    
    /// @brief Get the raw pointer
    /// @return RawPtr<T> The raw pointer
    inline RawPtr<T> Get() const noexcept {
        return m_tptr;
    }

    /// @brief Reset the shared pointer
    inline void Reset() noexcept {
        if (m_cb) {
            m_cb->Unref();
            m_tptr = nullptr;
            m_cb = nullptr;
        }
    }

    /// @brief Get the use count of the shared pointer
    inline size_t UseCount() const noexcept {
        return m_cb ? size_t(m_cb->shared_count) : 0;
    }

    /// @brief Swap the shared pointer with another
    inline void Swap(SharedPtr<T>& other) noexcept {
        std::swap(m_tptr, other.m_tptr);
        std::swap(m_cb, other.m_cb);
    }

    // Cast methods
    /// @brief Static cast the pointer
    template <typename U>
    requires DerivedFrom<U, T> || DerivedFrom<T, U> || OneOf<void, T, U>
    inline SharedPtr<U> Cast() const noexcept {
        return SharedPtr<U>(m_tptr.template Cast<U>(), m_cb);
    }

    /// @brief Dynamic cast the pointer
    template <typename U>
    inline SharedPtr<U> DynamicCast() const noexcept {
        auto realPtr = m_tptr.template DynamicCast<U>();
        if (realPtr) {
            return SharedPtr<U>(realPtr, m_cb);
        }
        return SharedPtr<U>(nullptr);
    }

    /// @brief Const cast the pointer
    template <typename U>
    requires Same<RemoveCV<T>, RemoveCV<U>>
    inline SharedPtr<U> ConstCast() const noexcept {
        return SharedPtr<U>(m_tptr.template ConstCast<U>(), m_cb);
    }

    /// @brief Reinterpret cast the pointer
    template <typename U>
    inline SharedPtr<U> ReinterpretCast() const noexcept {
        return SharedPtr<U>(m_tptr.template ReinterpretCast<U>(), m_cb);
    }
};

/// @brief Weak pointer
template <typename T>
class WeakPtr {
    template <typename U>
    friend class WeakPtr;
    template <typename U>
    friend class SharedPtr;
protected:
    RawPtr<T> m_tptr;
    RawPtr<_detail::ControlBlockBase<>> m_cb = nullptr;
    inline WeakPtr(RawPtr<T> tptr, RawPtr<_detail::ControlBlockBase<>> cb): m_tptr(tptr), m_cb(cb) {
        if (m_cb) m_cb->WeakRef();
    }
public:
    // Types
    using Type = T;
    // factory methods
    inline WeakPtr() = default;
    inline WeakPtr(std::nullptr_t): m_tptr(nullptr), m_cb(nullptr) {}
    inline WeakPtr(const WeakPtr<T>& other) noexcept: m_tptr(other.m_tptr), m_cb(other.m_cb) {
        if (m_cb) m_cb->WeakRef();
    }
    inline WeakPtr(WeakPtr<T>&& other) noexcept: m_tptr(std::move(other.m_tptr)), m_cb(std::move(other.m_cb)) {
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
    }
    inline WeakPtr(const SharedPtr<T>& other) noexcept: m_tptr(other.m_tptr), m_cb(other.m_cb) {
        if (m_cb) m_cb->WeakRef();
    }
    template <typename U>
    requires DerivedFrom<U, T>
    inline WeakPtr(const WeakPtr<U>& other) noexcept: m_tptr(other.m_tptr.template Cast<T>()), m_cb(other.m_cb) {
        if (m_cb) m_cb->WeakRef();
    }
    template <typename U>
    requires DerivedFrom<U, T> || Same<T, void>
    inline WeakPtr(WeakPtr<U>&& other) noexcept: m_tptr(other.m_tptr.template Cast<T>()), m_cb(other.m_cb) {
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
    }
    template <typename U>
    requires DerivedFrom<U, T> || Same<T, void>
    inline WeakPtr(const SharedPtr<U>& other) noexcept: m_tptr(other.m_tptr.template Cast<T>()), m_cb(other.m_cb) {
        if (m_cb) m_cb->WeakRef();
    }
    inline ~WeakPtr() {
        if (m_cb) {
            m_cb->WeakUnref();
            m_cb = nullptr; // Prevent dangling pointer
            m_tptr = nullptr;
        }
    }
    // operators
    inline WeakPtr<T>& operator=(const WeakPtr<T>& other) noexcept {
        if (this != &other) {
            if (m_cb) m_cb->WeakUnref();
            m_tptr = other.m_tptr;
            m_cb = other.m_cb;
            if (m_cb) m_cb->WeakRef();
        }
        return *this;
    }
    inline WeakPtr<T>& operator=(WeakPtr<T>&& other) noexcept {
        if (this != &other) {
            if (m_cb) m_cb->WeakUnref();
            m_tptr = std::move(other.m_tptr);
            m_cb = std::move(other.m_cb);
            other.m_tptr = nullptr;
            other.m_cb = nullptr;
        }
        return *this;
    }
    template <typename U>
    requires (DerivedFrom<U, T> && !Same<U, T>) || Same<T, void>
    inline WeakPtr<T>& operator=(const WeakPtr<U>& other) noexcept {
        if (m_cb) m_cb->WeakUnref();
        m_tptr = other.m_tptr.template Cast<T>();
        m_cb = other.m_cb;
        if (m_cb) m_cb->WeakRef();
        return *this;
    }
    template <typename U>
    requires (DerivedFrom<U, T> && !Same<U, T>) || Same<T, void>
    inline WeakPtr<T>& operator=(WeakPtr<U>&& other) noexcept {
        if (m_cb) m_cb->WeakUnref();
        m_tptr = other.m_tptr.template Cast<T>();
        m_cb = std::move(other.m_cb);
        other.m_tptr = nullptr;
        other.m_cb = nullptr;
        return *this;
    }
    template <typename U>
    requires DerivedFrom<U, T>  || Same<T, void>
    inline WeakPtr<T>& operator=(const SharedPtr<U>& other) noexcept {
        if (m_cb) m_cb->WeakUnref();
        m_tptr = other.m_tptr.template Cast<T>();
        m_cb = other.m_cb;
        if (m_cb) m_cb->WeakRef();
        return *this;
    }

    // Interface methods
    inline bool Expired() const noexcept {
        return !m_cb || m_cb->shared_count == 0;
    }
    
    inline SharedPtr<T> Lock() const noexcept {
        if (Expired()) {
            return SharedPtr<T>(nullptr);
        }
        return SharedPtr<T>(m_tptr, m_cb);
    }

    inline constexpr bool IsConst() const noexcept { return std::is_const_v<T>; }

    inline size_t UseCount() const noexcept {
        return m_cb ? size_t(m_cb->shared_count) : 0;
    }
};

/// @brief SharedPtr alias
template <typename T>
using Ptr = SharedPtr<T>;

template <typename T, typename... Args>
inline SharedPtr<T> MakePtr(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
};

template <typename T, typename... Args>
inline SharedPtr<T> New(Args&&... args) {
    return MakePtr<T>(std::forward<Args>(args)...);
};

/// @brief Unique pointer
/// @tparam T The type of the pointer
/// @tparam Deleter The deleter of the pointer
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePtr {
    template <typename U, typename D>
    friend class UniquePtr;
protected:
    RawPtr<T> ptr = nullptr;
    Deleter deleter;
public:
    using Type = T;
    
    // factory methods
    inline UniquePtr() = default;
    inline UniquePtr(std::nullptr_t): ptr(nullptr) {}
    inline UniquePtr(RawPtr<T> p): ptr(p) {}
    inline UniquePtr(RawPtr<T> p, Deleter d): ptr(p), deleter(std::move(d)) {}
    inline UniquePtr(const UniquePtr<T, Deleter>& other) = delete; // No copy allowed
    inline UniquePtr(UniquePtr<T, Deleter>&& other) noexcept: ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }
    inline ~UniquePtr() { Reset(); }

    // operators
    inline UniquePtr<T, Deleter>& operator=(const UniquePtr<T, Deleter>& other) = delete; // No copy allowed
    inline UniquePtr<T, Deleter>& operator=(UniquePtr<T, Deleter>&& other) noexcept {
        if (this != &other) {
            Reset();
            ptr = other.ptr;
            deleter = std::move(other.deleter);
            other.ptr = nullptr;
        }
        return *this;
    }

    inline T* operator->() const noexcept {
        _LCORE_CHECK_PTR_NOTZERO(ptr.Get());
        return ptr.operator->();
    }
    inline auto operator*() const noexcept requires (!Void<T>) {
        _LCORE_CHECK_PTR_NOTZERO(ptr.Get());
        return *ptr;
    }

    inline operator bool() const noexcept { return ptr != nullptr; }
    inline int operator<=>(const UniquePtr<T, Deleter>& other) const noexcept {
        return ptr <=> other.ptr;
    }

    // Interface methods
    inline constexpr bool IsConst() const noexcept { return std::is_const_v<T>; }
    
    inline RawPtr<T> Get() const noexcept { return ptr; }
    
    inline void Reset() noexcept {
        if (ptr) {
            deleter(ptr.Get());   // Call the deleter
            ptr = nullptr;  // Set the pointer to nullptr
        }
    }

    inline void Swap(UniquePtr<T, Deleter>& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(deleter, other.deleter);
    }

    /// @brief Release the ownership of the pointer and return it
    inline RawPtr<T> Release() noexcept {
        auto temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Cast methods
    // We don't implement Cast, DynamicCast, ConstCast, or ReinterpretCast for UniquePtr
    // because UniquePtr is not meant to be used polymorphically like RawPtr, SharedPtr.
    // If you need to cast, consider using SharedPtr or RawPtr instead.
    // If you really need to cast, you can use Get() to get the raw pointer and then cast it.
};

template <typename T, typename... Args>
inline constexpr UniquePtr<T> MakeUniquePtr(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
};

LCORE_NAMESPACE_END
