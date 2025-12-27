// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_SMART_PTR_H
#define EXS_SMART_PTR_H

#include "../Allocator/AllocatorBase.h"
#include "../../Core/Traits/Traits.h"
#include "../../Core/Utils/Utils.h"
#include <atomic>

namespace Exs_ {
namespace Memory {

// Forward declarations
template<typename T>
class ExsUniquePtr;

template<typename T>
class ExsSharedPtr;

template<typename T>
class ExsWeakPtr;

// Exs_ default deleter
template<typename T>
struct ExsDefaultDelete {
    constexpr ExsDefaultDelete() noexcept = default;
    
    template<typename U>
    ExsDefaultDelete(const ExsDefaultDelete<U>&) noexcept {}
    
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "Cannot delete incomplete type");
        delete ptr;
    }
};

// Exs_ array deleter
template<typename T>
struct ExsArrayDelete {
    constexpr ExsArrayDelete() noexcept = default;
    
    template<typename U>
    ExsArrayDelete(const ExsArrayDelete<U>&) noexcept {}
    
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "Cannot delete incomplete type");
        delete[] ptr;
    }
};

// Exs_ allocator deleter
template<typename T, typename Allocator = ExsIAllocator*>
struct ExsAllocatorDelete {
    Allocator allocator;
    
    explicit ExsAllocatorDelete(Allocator alloc = nullptr) noexcept 
        : allocator(alloc) {}
    
    template<typename U>
    ExsAllocatorDelete(const ExsAllocatorDelete<U, Allocator>& other) noexcept 
        : allocator(other.allocator) {}
    
    void operator()(T* ptr) const noexcept {
        if (ptr) {
            ptr->~T();
            if (allocator) {
                allocator->ExsDeallocate(ptr);
            }
        }
    }
};

// Exs_ control block for shared pointers
class EXS_API_HIDDEN ExsControlBlockBase {
protected:
    std::atomic<size_t> sharedCount;
    std::atomic<size_t> weakCount;
    
public:
    ExsControlBlockBase() noexcept 
        : sharedCount(1), weakCount(0) {}
    
    virtual ~ExsControlBlockBase() = default;
    
    virtual void ExsDestroy() noexcept = 0;
    virtual void ExsDeallocate() noexcept = 0;
    
    void ExsAddSharedRef() noexcept {
        sharedCount.fetch_add(1, std::memory_order_relaxed);
    }
    
    void ExsReleaseShared() noexcept {
        if (sharedCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ExsDestroy();
            if (weakCount.load(std::memory_order_relaxed) == 0) {
                ExsDeallocate();
            }
        }
    }
    
    void ExsAddWeakRef() noexcept {
        weakCount.fetch_add(1, std::memory_order_relaxed);
    }
    
    void ExsReleaseWeak() noexcept {
        if (weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (sharedCount.load(std::memory_order_relaxed) == 0) {
                ExsDeallocate();
            }
        }
    }
    
    size_t ExsGetSharedCount() const noexcept {
        return sharedCount.load(std::memory_order_relaxed);
    }
    
    size_t ExsGetWeakCount() const noexcept {
        return weakCount.load(std::memory_order_relaxed);
    }
};

template<typename T, typename Deleter, typename Allocator>
class ExsControlBlock : public ExsControlBlockBase {
private:
    T* ptr;
    Deleter deleter;
    Allocator allocator;
    
public:
    ExsControlBlock(T* p, Deleter d, Allocator a) noexcept
        : ptr(p), deleter(ExsMove(d)), allocator(ExsMove(a)) {}
    
    ~ExsControlBlock() = default;
    
    void ExsDestroy() noexcept override {
        if (ptr) {
            deleter(ptr);
            ptr = nullptr;
        }
    }
    
    void ExsDeallocate() noexcept override {
        using ControlBlockAllocator = 
            typename std::allocator_traits<Allocator>::template rebind_alloc<ExsControlBlock>;
        
        ControlBlockAllocator cbAlloc(allocator);
        cbAlloc.destroy(this);
        cbAlloc.deallocate(this, 1);
    }
};

// Exs_ unique pointer
template<typename T, typename Deleter = ExsDefaultDelete<T>>
class ExsUniquePtr {
private:
    T* ptr;
    Deleter deleter;
    
    template<typename U, typename D>
    friend class ExsUniquePtr;
    
public:
    using element_type = T;
    using deleter_type = Deleter;
    using pointer = T*;
    
    // Constructors
    constexpr ExsUniquePtr() noexcept : ptr(nullptr), deleter() {}
    
    constexpr ExsUniquePtr(std::nullptr_t) noexcept : ptr(nullptr), deleter() {}
    
    explicit ExsUniquePtr(pointer p) noexcept : ptr(p), deleter() {}
    
    ExsUniquePtr(pointer p, typename std::conditional<
        std::is_reference<Deleter>::value,
        Deleter,
        const Deleter&
    >::type d) noexcept : ptr(p), deleter(d) {}
    
    ExsUniquePtr(pointer p, typename std::remove_reference<Deleter>::type&& d) noexcept
        : ptr(p), deleter(ExsMove(d)) {
        static_assert(!std::is_reference<Deleter>::value,
                     "Deleter must not be reference if constructed with rvalue");
    }
    
    // Move constructors
    ExsUniquePtr(ExsUniquePtr&& other) noexcept
        : ptr(other.ptr), deleter(ExsMove(other.deleter)) {
        other.ptr = nullptr;
    }
    
    template<typename U, typename E>
    ExsUniquePtr(ExsUniquePtr<U, E>&& other) noexcept
        : ptr(other.ptr), deleter(ExsMove(other.deleter)) {
        other.ptr = nullptr;
    }
    
    // Destructor
    ~ExsUniquePtr() {
        if (ptr) {
            deleter(ptr);
        }
    }
    
    // Move assignment
    ExsUniquePtr& operator=(ExsUniquePtr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr = other.ptr;
            deleter = ExsMove(other.deleter);
            other.ptr = nullptr;
        }
        return *this;
    }
    
    ExsUniquePtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }
    
    // Deleted copy operations
    ExsUniquePtr(const ExsUniquePtr&) = delete;
    ExsUniquePtr& operator=(const ExsUniquePtr&) = delete;
    
    // Modifiers
    pointer release() noexcept {
        pointer p = ptr;
        ptr = nullptr;
        return p;
    }
    
    void reset(pointer p = pointer()) noexcept {
        if (ptr) {
            deleter(ptr);
        }
        ptr = p;
    }
    
    void swap(ExsUniquePtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(deleter, other.deleter);
    }
    
    // Observers
    pointer get() const noexcept { return ptr; }
    Deleter& get_deleter() noexcept { return deleter; }
    const Deleter& get_deleter() const noexcept { return deleter; }
    
    explicit operator bool() const noexcept { return ptr != nullptr; }
    
    typename std::add_lvalue_reference<T>::type operator*() const {
        EXS_ASSERT(ptr != nullptr);
        return *ptr;
    }
    
    pointer operator->() const noexcept {
        EXS_ASSERT(ptr != nullptr);
        return ptr;
    }
    
    // Comparisons
    bool operator==(const ExsUniquePtr& other) const { return ptr == other.ptr; }
    bool operator!=(const ExsUniquePtr& other) const { return ptr != other.ptr; }
    bool operator<(const ExsUniquePtr& other) const { return ptr < other.ptr; }
    bool operator<=(const ExsUniquePtr& other) const { return ptr <= other.ptr; }
    bool operator>(const ExsUniquePtr& other) const { return ptr > other.ptr; }
    bool operator>=(const ExsUniquePtr& other) const { return ptr >= other.ptr; }
    
    bool operator==(std::nullptr_t) const { return ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return ptr != nullptr; }
};

// Exs_ shared pointer
template<typename T>
class ExsSharedPtr {
private:
    T* ptr;
    ExsControlBlockBase* controlBlock;
    
    template<typename U>
    friend class ExsSharedPtr;
    
    template<typename U>
    friend class ExsWeakPtr;
    
public:
    using element_type = T;
    using pointer = T*;
    
    // Constructors
    constexpr ExsSharedPtr() noexcept : ptr(nullptr), controlBlock(nullptr) {}
    
    constexpr ExsSharedPtr(std::nullptr_t) noexcept : ptr(nullptr), controlBlock(nullptr) {}
    
    template<typename U>
    explicit ExsSharedPtr(U* p) : ptr(p), controlBlock(nullptr) {
        try {
            using ControlBlockType = ExsControlBlock<U, ExsDefaultDelete<U>, std::allocator<U>>;
            using ControlBlockAllocator = std::allocator<ControlBlockType>;
            
            ControlBlockAllocator alloc;
            ControlBlockType* cb = alloc.allocate(1);
            ::new (cb) ControlBlockType(p, ExsDefaultDelete<U>(), alloc);
            
            controlBlock = cb;
        } catch (...) {
            delete p;
            throw;
        }
    }
    
    template<typename U, typename Deleter>
    ExsSharedPtr(U* p, Deleter d) : ptr(p), controlBlock(nullptr) {
        try {
            using ControlBlockType = ExsControlBlock<U, Deleter, std::allocator<U>>;
            using ControlBlockAllocator = std::allocator<ControlBlockType>;
            
            ControlBlockAllocator alloc;
            ControlBlockType* cb = alloc.allocate(1);
            ::new (cb) ControlBlockType(p, ExsMove(d), alloc);
            
            controlBlock = cb;
        } catch (...) {
            d(p);
            throw;
        }
    }
    
    template<typename U, typename Deleter, typename Allocator>
    ExsSharedPtr(U* p, Deleter d, Allocator alloc) : ptr(p), controlBlock(nullptr) {
        try {
            using ControlBlockType = ExsControlBlock<U, Deleter, Allocator>;
            using ControlBlockAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlockType>;
            
            ControlBlockAllocator cbAlloc(alloc);
            ControlBlockType* cb = cbAlloc.allocate(1);
            ::new (cb) ControlBlockType(p, ExsMove(d), ExsMove(alloc));
            
            controlBlock = cb;
        } catch (...) {
            d(p);
            throw;
        }
    }
    
    // Copy constructors
    ExsSharedPtr(const ExsSharedPtr& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->ExsAddSharedRef();
        }
    }
    
    template<typename U>
    ExsSharedPtr(const ExsSharedPtr<U>& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->ExsAddSharedRef();
        }
    }
    
    // Move constructors
    ExsSharedPtr(ExsSharedPtr&& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        other.ptr = nullptr;
        other.controlBlock = nullptr;
    }
    
    template<typename U>
    ExsSharedPtr(ExsSharedPtr<U>&& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        other.ptr = nullptr;
        other.controlBlock = nullptr;
    }
    
    // Destructor
    ~ExsSharedPtr() {
        if (controlBlock) {
            controlBlock->ExsReleaseShared();
        }
    }
    
    // Assignment operators
    ExsSharedPtr& operator=(const ExsSharedPtr& other) noexcept {
        if (this != &other) {
            ExsSharedPtr(other).swap(*this);
        }
        return *this;
    }
    
    template<typename U>
    ExsSharedPtr& operator=(const ExsSharedPtr<U>& other) noexcept {
        ExsSharedPtr(other).swap(*this);
        return *this;
    }
    
    ExsSharedPtr& operator=(ExsSharedPtr&& other) noexcept {
        if (this != &other) {
            ExsSharedPtr(ExsMove(other)).swap(*this);
        }
        return *this;
    }
    
    template<typename U>
    ExsSharedPtr& operator=(ExsSharedPtr<U>&& other) noexcept {
        ExsSharedPtr(ExsMove(other)).swap(*this);
        return *this;
    }
    
    ExsSharedPtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }
    
    // Modifiers
    void reset() noexcept {
        ExsSharedPtr().swap(*this);
    }
    
    template<typename U>
    void reset(U* p) {
        ExsSharedPtr(p).swap(*this);
    }
    
    template<typename U, typename Deleter>
    void reset(U* p, Deleter d) {
        ExsSharedPtr(p, d).swap(*this);
    }
    
    template<typename U, typename Deleter, typename Allocator>
    void reset(U* p, Deleter d, Allocator alloc) {
        ExsSharedPtr(p, d, alloc).swap(*this);
    }
    
    void swap(ExsSharedPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(controlBlock, other.controlBlock);
    }
    
    // Observers
    T* get() const noexcept { return ptr; }
    
    T& operator*() const noexcept {
        EXS_ASSERT(ptr != nullptr);
        return *ptr;
    }
    
    T* operator->() const noexcept {
        EXS_ASSERT(ptr != nullptr);
        return ptr;
    }
    
    explicit operator bool() const noexcept { return ptr != nullptr; }
    
    size_t use_count() const noexcept {
        return controlBlock ? controlBlock->ExsGetSharedCount() : 0;
    }
    
    bool unique() const noexcept {
        return use_count() == 1;
    }
    
    // Comparisons
    template<typename U>
    bool owner_before(const ExsSharedPtr<U>& other) const noexcept {
        return controlBlock < other.controlBlock;
    }
    
    // Factory functions
    template<typename U, typename... Args>
    friend ExsSharedPtr<U> ExsMakeShared(Args&&... args);
};

// Exs_ weak pointer
template<typename T>
class ExsWeakPtr {
private:
    T* ptr;
    ExsControlBlockBase* controlBlock;
    
    template<typename U>
    friend class ExsWeakPtr;
    
    template<typename U>
    friend class ExsSharedPtr;
    
public:
    using element_type = T;
    
    // Constructors
    constexpr ExsWeakPtr() noexcept : ptr(nullptr), controlBlock(nullptr) {}
    
    ExsWeakPtr(const ExsWeakPtr& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->ExsAddWeakRef();
        }
    }
    
    template<typename U>
    ExsWeakPtr(const ExsWeakPtr<U>& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->ExsAddWeakRef();
        }
    }
    
    ExsWeakPtr(const ExsSharedPtr<T>& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->ExsAddWeakRef();
        }
    }
    
    // Move constructors
    ExsWeakPtr(ExsWeakPtr&& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        other.ptr = nullptr;
        other.controlBlock = nullptr;
    }
    
    template<typename U>
    ExsWeakPtr(ExsWeakPtr<U>&& other) noexcept
        : ptr(other.ptr), controlBlock(other.controlBlock) {
        other.ptr = nullptr;
        other.controlBlock = nullptr;
    }
    
    // Destructor
    ~ExsWeakPtr() {
        if (controlBlock) {
            controlBlock->ExsReleaseWeak();
        }
    }
    
    // Assignment operators
    ExsWeakPtr& operator=(const ExsWeakPtr& other) noexcept {
        if (this != &other) {
            ExsWeakPtr(other).swap(*this);
        }
        return *this;
    }
    
    template<typename U>
    ExsWeakPtr& operator=(const ExsWeakPtr<U>& other) noexcept {
        ExsWeakPtr(other).swap(*this);
        return *this;
    }
    
    ExsWeakPtr& operator=(const ExsSharedPtr<T>& other) noexcept {
        ExsWeakPtr(other).swap(*this);
        return *this;
    }
    
    ExsWeakPtr& operator=(ExsWeakPtr&& other) noexcept {
        if (this != &other) {
            ExsWeakPtr(ExsMove(other)).swap(*this);
        }
        return *this;
    }
    
    template<typename U>
    ExsWeakPtr& operator=(ExsWeakPtr<U>&& other) noexcept {
        ExsWeakPtr(ExsMove(other)).swap(*this);
        return *this;
    }
    
    // Modifiers
    void reset() noexcept {
        ExsWeakPtr().swap(*this);
    }
    
    void swap(ExsWeakPtr& other) noexcept {
        using std::swap;
        swap(ptr, other.ptr);
        swap(controlBlock, other.controlBlock);
    }
    
    // Observers
    size_t use_count() const noexcept {
        return controlBlock ? controlBlock->ExsGetSharedCount() : 0;
    }
    
    bool expired() const noexcept {
        return use_count() == 0;
    }
    
    ExsSharedPtr<T> lock() const noexcept {
        if (expired()) {
            return ExsSharedPtr<T>();
        }
        
        ExsSharedPtr<T> result;
        result.ptr = ptr;
        result.controlBlock = controlBlock;
        
        if (controlBlock) {
            controlBlock->ExsAddSharedRef();
        }
        
        return result;
    }
    
    // Comparisons
    template<typename U>
    bool owner_before(const ExsWeakPtr<U>& other) const noexcept {
        return controlBlock < other.controlBlock;
    }
    
    template<typename U>
    bool owner_before(const ExsSharedPtr<U>& other) const noexcept {
        return controlBlock < other.controlBlock;
    }
};

// Exs_ factory functions
template<typename T, typename... Args>
ExsSharedPtr<T> ExsMakeShared(Args&&... args) {
    using ControlBlockType = ExsControlBlock<T, ExsDefaultDelete<T>, std::allocator<T>>;
    using ControlBlockAllocator = std::allocator<ControlBlockType>;
    
    ControlBlockAllocator alloc;
    
    // Allocate memory for object and control block together
    struct CombinedBlock {
        ControlBlockType controlBlock;
        alignas(T) unsigned char object[sizeof(T)];
    };
    
    CombinedBlock* combined = reinterpret_cast<CombinedBlock*>(alloc.allocate(sizeof(CombinedBlock)));
    
    try {
        // Construct object
        T* objectPtr = reinterpret_cast<T*>(combined->object);
        ::new (objectPtr) T(std::forward<Args>(args)...);
        
        // Construct control block
        ::new (&combined->controlBlock) ControlBlockType(
            objectPtr, 
            ExsDefaultDelete<T>(), 
            alloc
        );
        
        // Create shared pointer
        ExsSharedPtr<T> result;
        result.ptr = objectPtr;
        result.controlBlock = &combined->controlBlock;
        
        return result;
    } catch (...) {
        alloc.deallocate(combined, sizeof(CombinedBlock));
        throw;
    }
}

template<typename T, typename Allocator, typename... Args>
ExsSharedPtr<T> ExsAllocateShared(const Allocator& alloc, Args&&... args) {
    using ControlBlockType = ExsControlBlock<T, ExsDefaultDelete<T>, Allocator>;
    using ControlBlockAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlockType>;
    
    ControlBlockAllocator cbAlloc(alloc);
    
    struct CombinedBlock {
        ControlBlockType controlBlock;
        alignas(T) unsigned char object[sizeof(T)];
    };
    
    CombinedBlock* combined = cbAlloc.allocate(sizeof(CombinedBlock));
    
    try {
        // Construct object
        T* objectPtr = reinterpret_cast<T*>(combined->object);
        ::new (objectPtr) T(std::forward<Args>(args)...);
        
        // Construct control block
        ::new (&combined->controlBlock) ControlBlockType(
            objectPtr, 
            ExsDefaultDelete<T>(), 
            alloc
        );
        
        // Create shared pointer
        ExsSharedPtr<T> result;
        result.ptr = objectPtr;
        result.controlBlock = &combined->controlBlock;
        
        return result;
    } catch (...) {
        cbAlloc.deallocate(combined, sizeof(CombinedBlock));
        throw;
    }
}

} // namespace Memory
} // namespace Exs_

#endif // EXS_SMART_PTR_H
