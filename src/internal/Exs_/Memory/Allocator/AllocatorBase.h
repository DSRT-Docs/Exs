// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_ALLOCATOR_BASE_H
#define EXS_ALLOCATOR_BASE_H

#include "../../Core/Macros.h"
#include "../../Core/Debug/Assert.h"
#include <cstddef>
#include <cstdint>

namespace Exs_ {
namespace Memory {

// Exs_ memory allocation statistics
struct ExsAllocationStats {
    size_t totalAllocated;
    size_t totalFreed;
    size_t peakAllocated;
    size_t allocationCount;
    size_t freeCount;
    size_t failedAllocations;
    
    constexpr size_t ExsGetCurrentUsage() const noexcept {
        return totalAllocated - totalFreed;
    }
    
    constexpr float ExsGetFragmentation() const noexcept {
        return allocationCount > 0 ? 
            (static_cast<float>(freeCount) / allocationCount) * 100.0f : 0.0f;
    }
    
    constexpr float ExsGetUtilization() const noexcept {
        return totalAllocated > 0 ? 
            (static_cast<float>(totalAllocated - totalFreed) / totalAllocated) * 100.0f : 0.0f;
    }
    
    void ExsReset() noexcept {
        totalAllocated = 0;
        totalFreed = 0;
        peakAllocated = 0;
        allocationCount = 0;
        freeCount = 0;
        failedAllocations = 0;
    }
};

// Exs_ memory allocation info
struct ExsAllocationInfo {
    void* ptr;
    size_t size;
    size_t alignment;
    const char* file;
    int line;
    uint64_t timestamp;
    uint32_t allocationId;
    
    static constexpr uint32_t EXS_ALLOCATION_MAGIC = 0x45585341; // "EXSA"
};

// Exs_ base allocator interface
class EXS_NODISCARD ExsIAllocator {
public:
    virtual ~ExsIAllocator() = default;
    
    // Core allocation methods
    virtual void* ExsAllocate(size_t size, size_t alignment = 8) = 0;
    virtual void ExsDeallocate(void* ptr) = 0;
    virtual void* ExsReallocate(void* ptr, size_t newSize, size_t alignment = 8) = 0;
    
    // Statistics
    virtual ExsAllocationStats ExsGetStats() const = 0;
    virtual void ExsResetStats() = 0;
    
    // Information
    virtual const char* ExsGetName() const = 0;
    virtual size_t ExsGetTotalSize() const = 0;
    virtual size_t ExsGetUsedSize() const = 0;
    virtual size_t ExsGetFreeSize() const = 0;
    
    // Validation
    virtual bool ExsOwnsPointer(const void* ptr) const = 0;
    virtual bool ExsIsValidPointer(const void* ptr) const = 0;
    virtual bool ExsCanAllocate(size_t size, size_t alignment = 8) const = 0;
    
    // Debugging
    virtual void ExsDumpMemoryMap() const = 0;
    virtual void ExsValidate() const = 0;
    
    // Configuration
    virtual void ExsSetName(const char* name) = 0;
    virtual void ExsSetThreadSafe(bool threadSafe) = 0;
    virtual bool ExsIsThreadSafe() const = 0;
    
protected:
    ExsIAllocator() = default;
    
    // Prevent copying
    ExsIAllocator(const ExsIAllocator&) = delete;
    ExsIAllocator& operator=(const ExsIAllocator&) = delete;
    
    // Allow moving
    ExsIAllocator(ExsIAllocator&&) = default;
    ExsIAllocator& operator=(ExsIAllocator&&) = default;
};

// Exs_ allocator traits
template<typename Allocator>
struct ExsAllocatorTraits {
    using value_type = typename Allocator::value_type;
    using pointer = typename Allocator::pointer;
    using const_pointer = typename Allocator::const_pointer;
    using size_type = typename Allocator::size_type;
    using difference_type = typename Allocator::difference_type;
    
    template<typename U>
    struct rebind {
        using other = typename Allocator::template rebind<U>::other;
    };
    
    static pointer allocate(Allocator& alloc, size_type n) {
        return alloc.allocate(n);
    }
    
    static void deallocate(Allocator& alloc, pointer p, size_type n) {
        alloc.deallocate(p, n);
    }
    
    template<typename U, typename... Args>
    static void construct(Allocator& alloc, U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    static void destroy(Allocator& alloc, U* p) {
        p->~U();
    }
    
    static size_type max_size(const Allocator& alloc) noexcept {
        return alloc.max_size();
    }
    
    static Allocator select_on_container_copy_construction(const Allocator& alloc) {
        return alloc;
    }
};

// Exs_ memory utilities
namespace ExsMemoryUtils {
    
    // Alignment utilities
    constexpr size_t ExsAlignForward(size_t ptr, size_t alignment) noexcept {
        EXS_ASSERT(EXS_IS_POWER_OF_TWO(alignment));
        return (ptr + alignment - 1) & ~(alignment - 1);
    }
    
    constexpr size_t ExsAlignBackward(size_t ptr, size_t alignment) noexcept {
        EXS_ASSERT(EXS_IS_POWER_OF_TWO(alignment));
        return ptr & ~(alignment - 1);
    }
    
    constexpr bool ExsIsAligned(size_t ptr, size_t alignment) noexcept {
        EXS_ASSERT(EXS_IS_POWER_OF_TWO(alignment));
        return (ptr & (alignment - 1)) == 0;
    }
    
    constexpr bool ExsIsAligned(const void* ptr, size_t alignment) noexcept {
        return ExsIsAligned(reinterpret_cast<size_t>(ptr), alignment);
    }
    
    constexpr size_t ExsCalculatePadding(size_t ptr, size_t alignment) noexcept {
        EXS_ASSERT(EXS_IS_POWER_OF_TWO(alignment));
        size_t alignedPtr = ExsAlignForward(ptr, alignment);
        return alignedPtr - ptr;
    }
    
    constexpr size_t ExsCalculatePaddingWithHeader(
        size_t ptr, 
        size_t alignment, 
        size_t headerSize
    ) noexcept {
        EXS_ASSERT(EXS_IS_POWER_OF_TWO(alignment));
        
        size_t padding = ExsCalculatePadding(ptr, alignment);
        size_t neededSpace = headerSize;
        
        if (padding < neededSpace) {
            neededSpace -= padding;
            padding += alignment * ((neededSpace + alignment - 1) / alignment);
        }
        
        return padding;
    }
    
    // Pointer utilities
    template<typename T>
    constexpr T* ExsAlignPointer(T* ptr, size_t alignment) noexcept {
        return reinterpret_cast<T*>(
            ExsAlignForward(reinterpret_cast<size_t>(ptr), alignment)
        );
    }
    
    template<typename T>
    constexpr const T* ExsAlignPointer(const T* ptr, size_t alignment) noexcept {
        return reinterpret_cast<const T*>(
            ExsAlignForward(reinterpret_cast<size_t>(ptr), alignment)
        );
    }
    
    template<typename T>
    constexpr T* ExsAddOffset(T* ptr, size_t offset) noexcept {
        return reinterpret_cast<T*>(
            reinterpret_cast<uint8_t*>(ptr) + offset
        );
    }
    
    template<typename T>
    constexpr const T* ExsAddOffset(const T* ptr, size_t offset) noexcept {
        return reinterpret_cast<const T*>(
            reinterpret_cast<const uint8_t*>(ptr) + offset
        );
    }
    
    template<typename T>
    constexpr T* ExsSubtractOffset(T* ptr, size_t offset) noexcept {
        return reinterpret_cast<T*>(
            reinterpret_cast<uint8_t*>(ptr) - offset
        );
    }
    
    template<typename T>
    constexpr const T* ExsSubtractOffset(const T* ptr, size_t offset) noexcept {
        return reinterpret_cast<const T*>(
            reinterpret_cast<const uint8_t*>(ptr) - offset
        );
    }
    
    // Memory operations
    void* EXS_FORCE_INLINE ExsMemCopy(void* dest, const void* src, size_t n) noexcept {
        if (EXS_UNLIKELY(dest == nullptr || src == nullptr || n == 0)) {
            return dest;
        }
        
        uint8_t* d = static_cast<uint8_t*>(dest);
        const uint8_t* s = static_cast<const uint8_t*>(src);
        
        // Simple byte-by-byte copy
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
        
        return dest;
    }
    
    void* EXS_FORCE_INLINE ExsMemMove(void* dest, const void* src, size_t n) noexcept {
        if (EXS_UNLIKELY(dest == nullptr || src == nullptr || n == 0)) {
            return dest;
        }
        
        uint8_t* d = static_cast<uint8_t*>(dest);
        const uint8_t* s = static_cast<const uint8_t*>(src);
        
        if (d < s) {
            // Copy forward
            for (size_t i = 0; i < n; ++i) {
                d[i] = s[i];
            }
        } else if (d > s) {
            // Copy backward
            for (size_t i = n; i > 0; --i) {
                d[i - 1] = s[i - 1];
            }
        }
        
        return dest;
    }
    
    void* EXS_FORCE_INLINE ExsMemSet(void* dest, int value, size_t n) noexcept {
        if (EXS_UNLIKELY(dest == nullptr || n == 0)) {
            return dest;
        }
        
        uint8_t* d = static_cast<uint8_t*>(dest);
        uint8_t v = static_cast<uint8_t>(value);
        
        for (size_t i = 0; i < n; ++i) {
            d[i] = v;
        }
        
        return dest;
    }
    
    int EXS_FORCE_INLINE ExsMemCompare(const void* ptr1, const void* ptr2, size_t n) noexcept {
        if (EXS_UNLIKELY(ptr1 == nullptr || ptr2 == nullptr || n == 0)) {
            return 0;
        }
        
        const uint8_t* p1 = static_cast<const uint8_t*>(ptr1);
        const uint8_t* p2 = static_cast<const uint8_t*>(ptr2);
        
        for (size_t i = 0; i < n; ++i) {
            if (p1[i] != p2[i]) {
                return static_cast<int>(p1[i]) - static_cast<int>(p2[i]);
            }
        }
        
        return 0;
    }
    
    // Memory pattern utilities
    template<typename Pattern>
    void ExsFillPattern(void* dest, size_t size, Pattern pattern) noexcept {
        if (EXS_UNLIKELY(dest == nullptr || size == 0)) {
            return;
        }
        
        uint8_t* d = static_cast<uint8_t*>(dest);
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&pattern);
        const size_t patternSize = sizeof(Pattern);
        
        for (size_t i = 0; i < size; ++i) {
            d[i] = p[i % patternSize];
        }
    }
    
    template<typename Pattern>
    bool ExsCheckPattern(const void* ptr, size_t size, Pattern pattern) noexcept {
        if (EXS_UNLIKELY(ptr == nullptr || size == 0)) {
            return true;
        }
        
        const uint8_t* p = static_cast<const uint8_t*>(ptr);
        const uint8_t* patternPtr = reinterpret_cast<const uint8_t*>(&pattern);
        const size_t patternSize = sizeof(Pattern);
        
        for (size_t i = 0; i < size; ++i) {
            if (p[i] != patternPtr[i % patternSize]) {
                return false;
            }
        }
        
        return true;
    }
    
    // Memory allocation patterns
    enum class ExsMemoryPattern : uint8_t {
        Uninitialized = 0xCD,
        Freed = 0xDD,
        Alignment = 0xED,
        Guard = 0xFD,
        Allocated = 0xBD
    };
    
    void ExsFillUninitialized(void* ptr, size_t size) noexcept {
        ExsMemSet(ptr, static_cast<int>(ExsMemoryPattern::Uninitialized), size);
    }
    
    void ExsFillFreed(void* ptr, size_t size) noexcept {
        ExsMemSet(ptr, static_cast<int>(ExsMemoryPattern::Freed), size);
    }
    
    void ExsFillAlignment(void* ptr, size_t size) noexcept {
        ExsMemSet(ptr, static_cast<int>(ExsMemoryPattern::Alignment), size);
    }
    
    void ExsFillGuard(void* ptr, size_t size) noexcept {
        ExsMemSet(ptr, static_cast<int>(ExsMemoryPattern::Guard), size);
    }
    
    void ExsFillAllocated(void* ptr, size_t size) noexcept {
        ExsMemSet(ptr, static_cast<int>(ExsMemoryPattern::Allocated), size);
    }
    
    // Memory validation
    bool ExsValidateMemoryRange(const void* ptr, size_t size) noexcept;
    bool ExsValidatePointer(const void* ptr) noexcept;
    
    // Memory barrier
    void ExsMemoryBarrier() noexcept;
    
    // Cache control
    void ExsFlushCacheLine(const void* ptr) noexcept;
    void ExsPrefetch(const void* ptr) noexcept;
    
} // namespace ExsMemoryUtils

// Exs_ allocator adapter for STL
template<typename T>
class ExsAllocatorAdapter {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::false_type;
    
    template<typename U>
    struct rebind {
        using other = ExsAllocatorAdapter<U>;
    };
    
    ExsAllocatorAdapter(ExsIAllocator* allocator = nullptr) noexcept 
        : allocator(allocator) {}
    
    template<typename U>
    ExsAllocatorAdapter(const ExsAllocatorAdapter<U>& other) noexcept 
        : allocator(other.allocator) {}
    
    pointer allocate(size_type n) {
        if (allocator) {
            return static_cast<pointer>(allocator->ExsAllocate(n * sizeof(T), alignof(T)));
        }
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(pointer p, size_type n) {
        if (allocator) {
            allocator->ExsDeallocate(p);
        } else {
            ::operator delete(p);
        }
    }
    
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    size_type max_size() const noexcept {
        return size_type(-1) / sizeof(T);
    }
    
    ExsIAllocator* get_allocator() const noexcept { return allocator; }
    
    bool operator==(const ExsAllocatorAdapter& other) const noexcept {
        return allocator == other.allocator;
    }
    
    bool operator!=(const ExsAllocatorAdapter& other) const noexcept {
        return !(*this == other);
    }
    
private:
    ExsIAllocator* allocator;
    
    template<typename U>
    friend class ExsAllocatorAdapter;
};

} // namespace Memory
} // namespace Exs_

#endif // EXS_ALLOCATOR_BASE_H
