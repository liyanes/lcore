#pragma once
#include "base.hpp"
#include "ref.hpp"
#include "lcore/exception.hpp"

LCORE_NAMESPACE_BEGIN

class AnyCastException : public Exception {
public:
    AnyCastException(): Exception() {}
    inline const char* what() const noexcept override {
        return "Bad Any Cast";
    }
};

class Any {
    struct VTable {
        RawPtr<const std::type_info> type_;
        RawPtr<const LCORE_REFLECTION_NAMESPACE_NAME::ReflectionBase> reflection_;

        void (*destruct)(void*);
        void (*deallocate)(void*); // If the object is allocated on heap, this function deallocates it, otherwise it does nothing
        void* (*copy)(const void*);
        void (*inline_copy)(void*, const void*);
        void* (*move)(void*);
        void (*inline_move)(void*, void*);
        void (*swap)(void*, void*);
    };

    void* data_;
    const VTable* vtable_;

    static constexpr size_t BufferSize = 3 * sizeof(void*);
    alignas(max_align_t) unsigned char buffer[BufferSize]; // Small Buffer Optimization

    template <typename T>
    static constexpr bool IsSmallObject = sizeof(T) <= BufferSize && std::is_nothrow_move_constructible_v<T>;

    inline bool IsSMOCached() const { return data_ == reinterpret_cast<const void*>(buffer); }
public:
    Any(): data_(nullptr), vtable_(nullptr) {}
    Any(std::nullptr_t): Any() {}
    Any(const Any& other): data_(nullptr), vtable_(nullptr) {
        if (other.vtable_) {
            if (IsSMOCached()) {
                other.vtable_->inline_copy(buffer, other.buffer);
                data_ = buffer;
            } else {
                data_ = other.vtable_->copy(other.data_);
            }
            vtable_ = other.vtable_;
        }
    }
    Any(Any&& other) noexcept: data_(nullptr), vtable_(nullptr) {
        if (other.vtable_) {
            if (other.IsSMOCached()) {
                other.vtable_->inline_move(buffer, other.buffer);
                data_ = buffer;
            } else {
                data_ = other.vtable_->move(other.data_);
                other.data_ = nullptr;
            }
            vtable_ = other.vtable_;
            other.vtable_ = nullptr;
        }
    }
    template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>>>
    Any(T&& value): data_(nullptr), vtable_(nullptr) {
        using U = std::decay_t<T>;
        static_assert(!std::is_same_v<U, std::nullptr_t>, "Cannot construct Any from nullptr_t");
        static_assert(!std::is_reference_v<U>, "Cannot construct Any from reference type");
        static_assert(!std::is_const_v<U>, "Cannot construct Any from const type");
        static_assert(!std::is_volatile_v<U>, "Cannot construct Any from volatile type");
        static_assert(std::is_move_constructible_v<U>, "Type must be move constructible");
        static_assert(std::is_copy_constructible_v<U>, "Type must be copy constructible");
        static_assert(std::is_destructible_v<U>, "Type must be destructible");
        static_assert(std::is_nothrow_move_constructible_v<U> || IsSmallObject<U>, "Type must be nothrow move constructible or small enough for SBO");
        static_assert(alignof(U) <= alignof(std::max_align_t), "Type alignment is too large");

        static const VTable vtable = {
            &typeid(U),
            LCORE_REFLECTION_NAMESPACE_NAME::GetReflection<U>(),

            // Destruct
            [](void* ptr) { static_cast<U*>(ptr)->~U(); },
            // Deallocate
            [](void* ptr) { if (ptr != reinterpret_cast<void*>(buffer)) { operator delete(ptr); } },
            // Copy
            [](const void* ptr) -> void* { return new U(*static_cast<const U*>(ptr)); },
            // Inline Copy
            [](void* dest, const void* src) { new (dest) U(*static_cast<const U*>(src)); },
            // Move
            [](void* ptr) -> void* { return new U(std::move(*static_cast<U*>(ptr))); },
            // Inline Move
            [](void* dest, void* src) { new (dest) U(std::move(*static_cast<U*>(src))); static_cast<U*>(src)->~U();},
            // Swap
            [](void* a, void* b) { std::swap(*static_cast<U*>(a), *static_cast<U*>(b)); }
        };
        if constexpr (IsSmallObject<U>) {
            new (buffer) U(std::forward<T>(value));
            data_ = buffer;
        } else {
            data_ = new U(std::forward<T>(value));
        }
        vtable_ = &vtable;
    }
    ~Any() {
        if (vtable_) {
            vtable_->destruct(data_);
            vtable_->deallocate(data_);
        }
    }

    Any& operator=(const Any& other) {
        if (this != &other) {
            this->~Any();
            new (this) Any(other);
        }
        return *this;
    }
    Any& operator=(Any&& other) noexcept {
        if (this != &other) {
            this->~Any();
            new (this) Any(std::move(other));
        }
        return *this;
    }
    template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>>>
    Any& operator=(T&& value) {
        this->~Any();
        new (this) Any(std::forward<T>(value));
        return *this;
    }
    inline bool HasValue() const { return vtable_ != nullptr; }
    inline void Reset() {
        this->~Any();
        data_ = nullptr;
        vtable_ = nullptr;
    }

    inline RawPtr<const std::type_info> Type() const { return vtable_ ? vtable_->type_ : nullptr; }
    inline RawPtr<const LCORE_REFLECTION_NAMESPACE_NAME::ReflectionBase> Reflection() const { return vtable_ ? vtable_->reflection_ : nullptr; }
    template <typename T>
    inline bool Is() const { return vtable_ && (*vtable_->type_ == typeid(T)); }
    template <typename T>
    inline T& Cast() { if (!Is<T>()) throw AnyCastException(); return *reinterpret_cast<T*>(data_); }
    template <typename T>
    inline const T& Cast() const { if (!Is<T>()) throw AnyCastException(); return *reinterpret_cast<const T*>(data_); }
    template <typename T>
    inline T* TryCast() { return Is<T>() ? reinterpret_cast<T*>(data_) : nullptr; }
    template <typename T>
    inline const T* TryCast() const { return Is<T>() ? reinterpret_cast<const T*>(data_) : nullptr; }
    inline void Swap(Any& other) {
        if (this == &other) return;
        if (vtable_ == other.vtable_) {
            if (vtable_) {
                vtable_->swap(data_, other.data_);
            }
        } else {
            Any temp = std::move(*this);
            *this = std::move(other);
            other = std::move(temp);
        }
    }
    friend inline void swap(Any& a, Any& b) { a.Swap(b); }

    void* Data() { return data_; }
    const void* Data() const { return data_; }
};

LCORE_NAMESPACE_END

