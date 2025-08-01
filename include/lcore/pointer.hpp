#pragma once
#include <memory>
#include "base.hpp"
#include "traits.hpp"

LCORE_NAMESPACE_BEGIN

/// @brief Raw pointer
/// @tparam T The type of the pointer
template <typename T>
class RawPtr {
    template <typename U>
    friend class RawPtr;
protected:
    T* ptr = nullptr;
public:
    inline RawPtr() = default;
    inline RawPtr(T* ptr): ptr(ptr) {}
    inline RawPtr(std::nullptr_t): ptr(nullptr) {}
    inline RawPtr(const RawPtr<T>& ptr): ptr(ptr.ptr) {}
    inline RawPtr(RawPtr<T>&& ptr): ptr(ptr.ptr) {ptr.ptr = nullptr;}
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline RawPtr(const RawPtr<U>& ptr): ptr(ptr.ptr) {}

    inline RawPtr<T>& operator=(const RawPtr<T>& ptr) noexcept {this->ptr = ptr.ptr; return *this;}
    inline RawPtr<T>& operator=(RawPtr<T>&& ptr) noexcept {this->ptr = ptr.ptr; ptr.ptr = nullptr; return *this;}
    inline T* operator->() const noexcept {return ptr;}
    inline T& operator*() const noexcept {return *ptr;}
    inline operator bool() const noexcept {return ptr != nullptr;}
    inline bool operator==(const RawPtr<T>& ptr) const noexcept {return this->ptr == ptr.ptr;}
    inline bool operator==(std::nullptr_t) const noexcept {return ptr == nullptr;}
    inline bool operator!=(const RawPtr<T>& ptr) const noexcept {return this->ptr != ptr.ptr;}
    inline bool operator!=(std::nullptr_t) const noexcept {return ptr != nullptr;}
    inline bool operator<(const RawPtr<T>& ptr) const noexcept {return this->ptr < ptr.ptr;}
    inline bool operator>(const RawPtr<T>& ptr) const noexcept {return this->ptr > ptr.ptr;}
    inline bool operator<=(const RawPtr<T>& ptr) const noexcept {return this->ptr <= ptr.ptr;}
    inline bool operator>=(const RawPtr<T>& ptr) const noexcept {return this->ptr >= ptr.ptr;}

    inline constexpr bool IsConst() const noexcept {return std::is_const_v<T>;}
    inline T* Get() const noexcept {return ptr;}

    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr RawPtr<U> Cast() const noexcept {
        return static_cast<U*>(ptr);
    }

    template <typename U>
    inline RawPtr<U> DynamicCast() const noexcept {
        return dynamic_cast<U*>(ptr);
    }

    template <typename U>
    inline constexpr RawPtr<U> ConstCast() const noexcept {
        return const_cast<U*>(ptr);
    }

    template <typename U>
    inline RawPtr<U> ReinterpretCast() const noexcept {
        return reinterpret_cast<U*>(ptr);
    }

    inline void Delete() noexcept {
        delete ptr;
        ptr = nullptr;
    }
};

// operator delete for RawPtr
// template <typename T>
// inline void operator delete(RawPtr<T>& ptr) noexcept {
//     ptr.Delete();
// }

/// @brief Smart pointer
/// @tparam T The type of the pointer
template <typename T>
class Ptr: private std::shared_ptr<T> {
public:
    using std::shared_ptr<T>::shared_ptr;
    inline Ptr(std::shared_ptr<T>&& ptr): std::shared_ptr<T>(std::move(ptr)) {}
    
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr(const Ptr<U>& ptr): std::shared_ptr<T>(ptr) {};

    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr(Ptr<U>&& ptr): std::shared_ptr<T>(std::move(ptr)) {};

    /// @brief Static cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T> || IsDerivedFrom<T,U>
    inline Ptr<U> Cast() const noexcept {
        return std::static_pointer_cast<U>(*this);
    };

    /// @brief Dynamic cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer, if failed, return nullptr
    /// @note Dynamic cast is slower and should be avoided if possible
    template <typename U>
    requires IsDerivedFrom<U, T> || IsDerivedFrom<T,U>
    inline Ptr<U> DynamicCast() const noexcept {
        return std::dynamic_pointer_cast<U>(*this);
    };

    /// @brief Const cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T> || IsDerivedFrom<T,U>
    inline Ptr<U> ConstCast() const noexcept {
        return std::const_pointer_cast<U>(*this);
    };

    /// @brief Reinterpret cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T> || IsDerivedFrom<T,U>
    inline Ptr<U> ReinterpretCast() const noexcept {
        return std::reinterpret_pointer_cast<U>(*this);
    };

    /// @brief Get the raw pointer
    /// @return RawPtr<T> The raw pointer
    inline RawPtr<T> Get() const noexcept {
        return std::shared_ptr<T>::get();
    };
};

template <typename T, typename U>
inline Ptr<T> StaticCast(const Ptr<U>& ptr) noexcept {
    return std::static_pointer_cast<T>(ptr);
};

template <typename T, typename U>
inline Ptr<T> StaticCast(Ptr<U>&& ptr) noexcept {
    return std::static_pointer_cast<T>(std::forward<Ptr<U>>(ptr));
};

template <typename T, typename U>
inline Ptr<T> DynamicCast(const Ptr<U>& ptr) noexcept {
    return std::dynamic_pointer_cast<T>(ptr);
};

template <typename T, typename U>
inline Ptr<T> DynamicCast(Ptr<U>&& ptr) noexcept {
    return std::dynamic_pointer_cast<T>(std::forward<Ptr<U>>(ptr));
};

template <typename T, typename U>
inline Ptr<T> ConstCast(const Ptr<U>& ptr) noexcept {
    return std::const_pointer_cast<T>(ptr);
};

template <typename T, typename U>
inline Ptr<T> ConstCast(Ptr<U>&& ptr) noexcept {
    return std::const_pointer_cast<T>(std::forward<Ptr<U>>(ptr));
};

template <typename T, typename U>
inline Ptr<T> ReinterpretCast(const Ptr<U>& ptr) noexcept {
    return std::reinterpret_pointer_cast<T>(ptr);
};

template <typename T, typename U>
inline Ptr<T> ReinterpretCast(Ptr<U>&& ptr) noexcept {
    return std::reinterpret_pointer_cast<T>(std::forward<Ptr<U>>(ptr));
};

template <typename T, typename... Args>
inline Ptr<T> MakePtr(Args&&... args) {
    return Ptr<T>(new T(std::forward<Args>(args)...));
};

template <typename T, typename... Args>
inline Ptr<T> New(Args&&... args) {
    return MakePtr<T>(std::forward<Args>(args)...);
};

/// @brief Unique pointer
/// @tparam T The type of the pointer
/// @tparam Deleter The deleter of the pointer
template <typename T, typename Deleter = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Deleter>;

template <typename T, typename... Args>
inline constexpr UniquePtr<T> MakeUniquePtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
};

/// @brief Weak pointer
/// @tparam T The type of the pointer
template <typename T>
class WeakPtr: private std::weak_ptr<T> {
public:
    using std::weak_ptr<T>::weak_ptr;
    inline WeakPtr(std::weak_ptr<T>&& ptr): std::weak_ptr<T>(std::move(ptr)) {}
    
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr WeakPtr(const WeakPtr<U>& ptr): std::weak_ptr<T>(ptr) {};

    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr WeakPtr(WeakPtr<U>&& ptr): std::weak_ptr<T>(std::move(ptr)) {};

    inline Ptr<T> Lock() const noexcept {
        return std::weak_ptr<T>::lock();
    };

    /// @brief Static cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr<U> Cast() const noexcept {
        return std::static_pointer_cast<U>(std::weak_ptr<T>::lock());
    };

    /// @brief Dynamic cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer, if failed, return nullptr
    /// @note Dynamic cast is slower and should be avoided if possible
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr<U> DynamicCast() const noexcept {
        return std::dynamic_pointer_cast<U>(std::weak_ptr<T>::lock());
    };

    /// @brief Const cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr<U> ConstCast() const noexcept {
        return std::const_pointer_cast<U>(std::weak_ptr<T>::lock());
    };

    /// @brief Reinterpret cast the pointer
    /// @tparam U The type to cast to
    /// @return Ptr<U> The casted pointer
    template <typename U>
    requires IsDerivedFrom<U, T>
    inline constexpr Ptr<U> ReinterpretCast() const noexcept {
        return std::reinterpret_pointer_cast<U>(std::weak_ptr<T>::lock());
    };
};

LCORE_NAMESPACE_END
