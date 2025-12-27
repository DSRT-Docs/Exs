// Copyright 2024 Exs Framework. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");

#pragma once

#ifndef EXS_STACK_ALLOCATOR_H
#define EXS_STACK_ALLOCATOR_H

#include "AllocatorBase.h"
#include "../../Core/Debug/Debug.h"
#include <atomic>

namespace Exs_ {
namespace Memory {

// Exs_ stack allocator marker
struct ExsStackMarker {
    size_t offset;
    uint32_t markerId;
    uint32_t magic;
    
    static constexpr uint32_t EXS_STACK_MAGIC = 0x45585353; // "EXSS"
};

// Exs_ stack allocator for LIFO allocations
class ExsStackAllocator final : public ExsIAllocator {
private:
    // Configuration
    void* memoryBlock;
    size_t totalSize;
    size_t alignment;
    
    // Stack management
    std::atomic<size_t> currentOffset;
    std::atomic<uint32_t> markerCounter;
    
    // Statistics
    mutable std::atomic<ExsAllocationStats> stats;
    
    // Metadata
    char allocatorName[64];
    bool ownsMemory;
    bool threadSafe;
    
    // Mutex for thread safety
    mutable void* mutex;
    
    // Internal methods
    void* ExsAllocateInternal(size_t size, size_t alignment);
    void ExsLock() const;
    void ExsUnlock() const;
    
public:
    // Construction/Destruction
    ExsStackAllocator(
        size_t size, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsStackAllocator"
    );
    
    ExsStackAllocator(
        void* memory, 
        size_t size, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsStackAllocator"
    );
    
    ~ExsStackAllocator();
    
    // Non-copyable, non-movable
    ExsStackAllocator(const ExsStackAllocator&) = delete;
    ExsStackAllocator& operator=(const ExsStackAllocator&) = delete;
    ExsStackAllocator(ExsStackAllocator&&) = delete;
    ExsStackAllocator& operator=(ExsStackAllocator&&) = delete;
    
    // ExsIAllocator implementation
    void* ExsAllocate(size_t size, size_t alignment = 8) override;
    void ExsDeallocate(void* ptr) override;
    void* ExsReallocate(void* ptr, size_t newSize, size_t alignment = 8) override;
    
    ExsAllocationStats ExsGetStats() const override;
    void ExsResetStats() override;
    
    const char* ExsGetName() const override;
    size_t ExsGetTotalSize() const override;
    size_t ExsGetUsedSize() const override;
    size_t ExsGetFreeSize() const override;
    
    bool ExsOwnsPointer(const void* ptr) const override;
    bool ExsIsValidPointer(const void* ptr) const override;
    bool ExsCanAllocate(size_t size, size_t alignment = 8) const override;
    
    void ExsDumpMemoryMap() const override;
    void ExsValidate() const override;
    
    void ExsSetName(const char* name) override;
    void ExsSetThreadSafe(bool threadSafe) override;
    bool ExsIsThreadSafe() const override;
    
    // Stack-specific methods
    ExsStackMarker ExsGetMarker() const noexcept;
    void ExsRewindToMarker(ExsStackMarker marker);
    
    void ExsClear();
    void ExsRewind();
    
    size_t ExsGetAvailableSpace() const noexcept;
    size_t ExsGetUsedSpace() const noexcept;
    
    float ExsGetUtilization() const noexcept;
    
    bool ExsIsEmpty() const noexcept;
    bool ExsIsFull() const noexcept;
    
    // Factory methods
    static ExsStackAllocator* ExsCreate(
        size_t size, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsStackAllocator"
    );
    
    static ExsStackAllocator* ExsCreateFromMemory(
        void* memory, 
        size_t size, 
        size_t alignment = 8,
        bool threadSafe = true,
        const char* name = "ExsStackAllocator"
    );
    
    static void ExsDestroy(ExsStackAllocator* allocator);
    
private:
    // Allocation header
    struct ExsStackAllocationHeader {
        size_t size;
        size_t padding;
        uint32_t markerId;
        uint32_t magic;
        
        static constexpr uint32_t EXS_HEADER_MAGIC = 0x45585348; // "EXSH"
    };
    
    // Validation
    bool ExsIsValidHeader(const ExsStackAllocationHeader* header) const;
    bool ExsIsPointerInRange(const void* ptr) const;
    
    // Statistics helpers
    void ExsUpdateStatsAllocation(size_t size);
    void ExsUpdateStatsDeallocation(size_t size);
    
    // Debug
    void ExsFillWithPattern(void* ptr, size_t size, ExsMemoryUtils::ExsMemoryPattern pattern);
};

// Exs_ scoped stack allocator
class ExsScopedStackAllocator {
private:
    ExsStackAllocator* allocator;
    ExsStackMarker marker;
    
public:
    explicit ExsScopedStackAllocator(ExsStackAllocator* alloc)
        : allocator(alloc), marker(alloc->ExsGetMarker()) {}
    
    ~ExsScopedStackAllocator() {
        if (allocator) {
            allocator->ExsRewindToMarker(marker);
        }
    }
    
    // Non-copyable
    ExsScopedStackAllocator(const ExsScopedStackAllocator&) = delete;
    ExsScopedStackAllocator& operator=(const ExsScopedStackAllocator&) = delete;
    
    // Movable
    ExsScopedStackAllocator(ExsScopedStackAllocator&& other) noexcept 
        : allocator(other.allocator), marker(other.marker) {
        other.allocator = nullptr;
    }
    
    ExsScopedStackAllocator& operator=(ExsScopedStackAllocator&& other) noexcept {
        if (this != &other) {
            allocator = other.allocator;
            marker = other.marker;
            other.allocator = nullptr;
        }
        return *this;
    }
    
    ExsStackAllocator* ExsGetAllocator() const noexcept { return allocator; }
    ExsStackMarker ExsGetMarker() const noexcept { return marker; }
    
    void ExsRelease() { allocator = nullptr; }
};

} // namespace Memory
} // namespace Exs_

#endif // EXS_STACK_ALLOCATOR_H
